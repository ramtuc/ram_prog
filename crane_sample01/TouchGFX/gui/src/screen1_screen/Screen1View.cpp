#include <gui/screen1_screen/Screen1View.hpp>
#include "stm32u5xx_hal.h"
#include <main.h>
Screen1View::Screen1View()
{

}
void delayMicroseconds(uint32_t us)
{
    uint32_t count = us * 72; // 72MHzのシステムクロックに基づく計算
    for (volatile uint32_t i = 0; i < count; ++i)
    {
        __NOP();
    }
}
void Screen1View::setupScreen()
{
    Screen1ViewBase::setupScreen();
    Left_Button.getPressedState();
    Right_Button.getPressedState();
    Front_Button.getPressedState();
    Back_Button.getPressedState();
    Up_Button.getPressedState();
    Down_Button.getPressedState();
    Open_Button.getPressedState();
    Close_Button.getPressedState();

    // DRV8825ドライバの初期化（EN_DRVを低にしてドライバを有効にする）
    HAL_GPIO_WritePin(EN_DRV_GPIO_Port, EN_DRV_Pin, GPIO_PIN_RESET);
}

void Screen1View::tearDownScreen()
{
    Screen1ViewBase::tearDownScreen();
}

void Screen1View:: Func_Left_Button()
{
	HAL_GPIO_WritePin(outLeft_GPIO_Port, outLeft_Pin, GPIO_PIN_SET);
}

void Screen1View:: Func_Right_Button()
{

	HAL_GPIO_WritePin(outRight_GPIO_Port, outRight_Pin, GPIO_PIN_SET);
}

void Screen1View:: Func_Front_Button()
{
	HAL_GPIO_WritePin(outFront_GPIO_Port, outFront_Pin, GPIO_PIN_SET);
}

void Screen1View:: Func_Back_Button()
{
	HAL_GPIO_WritePin(outBack_GPIO_Port, outBack_Pin, GPIO_PIN_SET);
}

void Screen1View:: Func_Up_Button()
{
	HAL_GPIO_WritePin(outUp_GPIO_Port, outUp_Pin, GPIO_PIN_SET);
}

void Screen1View:: Func_Down_Button()
{
	HAL_GPIO_WritePin(outDown_GPIO_Port, outDown_Pin, GPIO_PIN_SET);
}

void Screen1View:: Func_Open_Button()
{
    // モーターを正転させる（DIRを設定してからSTEPにパルスを送る）
	 HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_SET);
	    for (int i = 0; i < 2; ++i)
	    {
	        HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
	        HAL_Delay(7);
	        //delayMicroseconds(250); // 1msの遅延
	        HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
	        //delayMicroseconds(250); // 1msの遅延
	        HAL_Delay(7);
	    }
}

void Screen1View:: Func_Close_Button()
{
	HAL_GPIO_WritePin(DIR_GPIO_Port, DIR_Pin, GPIO_PIN_RESET);
	    for (int i = 0; i < 2; ++i)
	    {
	        HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_SET);
	        HAL_Delay(7);
	        //delayMicroseconds(250); // 1msの遅延
	        HAL_GPIO_WritePin(STEP_GPIO_Port, STEP_Pin, GPIO_PIN_RESET);
	        HAL_Delay(7);
	        //delayMicroseconds(250); // 1msの遅延
	    }
}

void Screen1View::handleTickEvent()
{
    // Left_Buttonが押されているかどうかをチェック
    if (Left_Button.getPressedState())
    {
        Func_Left_Button(); // ボタンが押されている間、左のボタン関数を呼び出す
    }
    else
    {
        HAL_GPIO_WritePin(outLeft_GPIO_Port, outLeft_Pin, GPIO_PIN_RESET);
    }

    // Right_Buttonが押されているかどうかをチェック
    if (Right_Button.getPressedState())
    {
        Func_Right_Button(); // ボタンが押されている間、右のボタン関数を呼び出す
    }
    else
    {
        HAL_GPIO_WritePin(outRight_GPIO_Port, outRight_Pin, GPIO_PIN_RESET);
    }
    // Front_Buttonが押されているかどうかをチェック
	if (Front_Button.getPressedState())
	{
		Func_Front_Button(); // ボタンが押されている間、前のボタン関数を呼び出す
	}
	else
	{
		HAL_GPIO_WritePin(outFront_GPIO_Port, outFront_Pin, GPIO_PIN_RESET);
	}

	// Back_Buttonが押されているかどうかをチェック
	if (Back_Button.getPressedState())
	{
		Func_Back_Button(); // ボタンが押されている間、後ろのボタン関数を呼び出す
	}
	else
	{
		HAL_GPIO_WritePin(outBack_GPIO_Port, outBack_Pin, GPIO_PIN_RESET);
	}

	// Up_Buttonが押されているかどうかをチェック
	if (Up_Button.getPressedState())
	{
		Func_Up_Button(); // ボタンが押されている間、上のボタン関数を呼び出す
	}
	else
	{
		HAL_GPIO_WritePin(outUp_GPIO_Port, outUp_Pin, GPIO_PIN_RESET);
	}

	// Down_Buttonが押されているかどうかをチェック
	if (Down_Button.getPressedState())
	{
		Func_Down_Button(); // ボタンが押されている間、下のボタン関数を呼び出す
	}
	else
	{
		HAL_GPIO_WritePin(outDown_GPIO_Port, outDown_Pin, GPIO_PIN_RESET);
	}
	 if (Open_Button.getPressedState())
	    {
	        Func_Open_Button(); // ボタンが押されている間、開くボタン関数を呼び出す
	    }

	    // Close_Buttonが押されているかどうかをチェック
	    if (Close_Button.getPressedState())
	    {
	        Func_Close_Button(); // ボタンが押されている間、閉じるボタン関数を呼び出す
	    }
}



