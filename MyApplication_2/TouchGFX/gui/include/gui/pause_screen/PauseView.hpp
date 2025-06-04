#ifndef PAUSEVIEW_HPP
#define PAUSEVIEW_HPP

#include <gui_generated/pause_screen/PauseViewBase.hpp>
#include <gui/pause_screen/PausePresenter.hpp>

class PauseView : public PauseViewBase
{
public:
    PauseView();
    virtual ~PauseView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // PAUSEVIEW_HPP
