#ifndef INTROVIEW_HPP
#define INTROVIEW_HPP

#include <gui_generated/intro_screen/IntroViewBase.hpp>
#include <gui/intro_screen/IntroPresenter.hpp>

class IntroView : public IntroViewBase
{
public:
    IntroView();
    virtual ~IntroView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // INTROVIEW_HPP
