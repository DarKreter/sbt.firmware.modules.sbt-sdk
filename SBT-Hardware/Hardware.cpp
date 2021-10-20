#include <Hardware.hpp>

UART Hardware::uart1(USART1), Hardware::uart2(USART2), Hardware::uart3(USART3);
I2C Hardware::i2c1(I2C1), Hardware::i2c2(I2C2);
SPI_t Hardware::spi1(SPI1), Hardware::spi2(SPI2);
CAN Hardware::can;

void Hardware::configureClocks(uint32_t ahbFreq)
{
    //-----------HERE----------------------
    if(ahbFreq < 100'000 || 72'000'000 < ahbFreq)
        softfault(__FILE__, __LINE__, "Clock speed must be between 100'000 and 72'000'000");
    
    static const auto calculateBestAHBPrescalerAndPLLMUL = [ahbFreq]() -> std::pair<uint32_t, uint32_t>
    {
        // Calculate theoretical AHB frequency based on passed parameters. Return nullopt if frequency is higher than 72MHz
        static const auto calculateAHBFreq = [](uint32_t PLLMUL, uint32_t AHBPrescaler) -> std::optional<uint32_t> {
            constexpr uint32_t hseFreq = 8'000'000;
            constexpr uint32_t maxAHBFreq = 72'000'000;
            const uint32_t ahbFreq = hseFreq * PLLMUL / AHBPrescaler;
            if(ahbFreq <= maxAHBFreq)
                return ahbFreq;
            
            return std::nullopt;
        };
        
        constexpr std::array possiblePLLMUL = {
                std::make_pair(4,RCC_PLL_MUL4),
                std::make_pair(5,RCC_PLL_MUL5),
                std::make_pair(6,RCC_PLL_MUL6),
                std::make_pair(7,RCC_PLL_MUL7),
                std::make_pair(8,RCC_PLL_MUL8),
                std::make_pair(9,RCC_PLL_MUL9),
                std::make_pair(10,RCC_PLL_MUL10),
                std::make_pair(11,RCC_PLL_MUL11),
                std::make_pair(12,RCC_PLL_MUL12),
                std::make_pair(13,RCC_PLL_MUL13),
                std::make_pair(14,RCC_PLL_MUL14),
                std::make_pair(15,RCC_PLL_MUL15),
                std::make_pair(16,RCC_PLL_MUL16)};
        
        constexpr std::array possibleAHBPrescalers = {
                std::make_pair(1, RCC_SYSCLK_DIV1),
                std::make_pair(2, RCC_SYSCLK_DIV2),
                std::make_pair(4, RCC_SYSCLK_DIV4),
                std::make_pair(8, RCC_SYSCLK_DIV8),
                std::make_pair(16, RCC_SYSCLK_DIV16),
                std::make_pair(64, RCC_SYSCLK_DIV64),
                std::make_pair(128, RCC_SYSCLK_DIV128),
                std::make_pair(256, RCC_SYSCLK_DIV256),
                std::make_pair(512, RCC_SYSCLK_DIV512)};
        
        
        
        // Iterate over all combinations and determine the best (lowest error) option
        uint32_t bestPLLMUL = 0;
        uint32_t bestAHBPrescaler = 0;
        uint32_t lowestAbsoluteError = ahbFreq;
        
        for(const auto& [PLLMUL, PLLMULDefine] : possiblePLLMUL){
            for(const auto& [AHBPrescaler, AHBPrescalerDefine] : possibleAHBPrescalers){
                if(const auto calculatedFreq = calculateAHBFreq(PLLMUL, AHBPrescaler)){
                    
                    const uint32_t absoluteError = std::abs(static_cast<int>(calculatedFreq.value()) - static_cast<int>(ahbFreq));
                    if(absoluteError < lowestAbsoluteError)
                    {
                        bestPLLMUL = PLLMULDefine;
                        bestAHBPrescaler = AHBPrescalerDefine;
                        lowestAbsoluteError = absoluteError;
                    }
                    
                    // Do not calculate other values if we found perfect combo
                    if(lowestAbsoluteError == 0)
                        break;
                    
                }
            }
        }
        
        return {bestPLLMUL, bestAHBPrescaler};
    };
    
    RCC_ClkInitTypeDef clk{};
    RCC_OscInitTypeDef osc{};
    
    
    const auto [PLLMUL, AHBPrescaler] = calculateBestAHBPrescalerAndPLLMUL();
    
    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLMUL = PLLMUL;//(4 - 16);
    

    clk.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                    |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    // Use PLL for fine tuning of SYSCLK
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = AHBPrescaler;

    if(ahbFreq > 36'000'000)
        clk.APB1CLKDivider = RCC_HCLK_DIV2;
    else
        clk.APB1CLKDivider = RCC_HCLK_DIV1;
    
    clk.APB2CLKDivider = RCC_HCLK_DIV1;
    
    HAL_RCC_OscConfig(&osc);
    HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    
    
    // Enable clocks for GPIOs
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
}

void Hardware::enableGpio(GPIO_TypeDef* gpio, uint32_t pin, Gpio::Mode direction, Gpio::Pull pull)
{
    GPIO_InitTypeDef initTypeDef;
    initTypeDef.Pin = pin;
    switch (direction) {
        case Gpio::Mode::Input:
            initTypeDef.Mode = GPIO_MODE_INPUT;
            break;
        case Gpio::Mode::Output:
            initTypeDef.Mode = GPIO_MODE_OUTPUT_PP;
            break;
        case Gpio::Mode::AlternateInput:
            initTypeDef.Mode = GPIO_MODE_AF_INPUT;
            break;
        case Gpio::Mode::AlternatePP:
            initTypeDef.Mode = GPIO_MODE_AF_PP;
            break;
        case Gpio::Mode::AlternateOD:
            initTypeDef.Mode = GPIO_MODE_AF_OD;
            break;
    }
    initTypeDef.Pull = static_cast<uint32_t>(pull);
    initTypeDef.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(gpio, &initTypeDef);
}

void Hardware::toggle(GPIO_TypeDef* gpio, uint32_t pin)
{
    HAL_GPIO_TogglePin(gpio, pin);
}


// You did something wrong
void softfault([[maybe_unused]]const std::string& fileName, [[maybe_unused]]const int& lineNumber, [[maybe_unused]]const std::string& comment)
{
    while (true);
}