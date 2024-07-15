#ifndef SCREEN1VIEW_HPP
#define SCREEN1VIEW_HPP

#include <gui_generated/screen1_screen/Screen1ViewBase.hpp>
#include <gui/screen1_screen/Screen1Presenter.hpp>

class Screen1View : public Screen1ViewBase
{
public:
    Screen1View();
    virtual ~Screen1View() {}
    virtual void setupScreen();
    virtual void tearDownScreen();

    virtual void Func_Left_Button();
    virtual void Func_Right_Button();
    virtual void Func_Front_Button();
    virtual void Func_Back_Button();
    virtual void Func_Up_Button();
    virtual void Func_Down_Button();
    virtual void Func_Open_Button();
    virtual void Func_Close_Button();
    void handleTickEvent();
protected:
};

#endif // SCREEN1VIEW_HPP
