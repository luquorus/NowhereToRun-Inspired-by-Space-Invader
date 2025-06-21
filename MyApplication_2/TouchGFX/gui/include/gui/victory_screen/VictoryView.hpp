#ifndef VICTORYVIEW_HPP
#define VICTORYVIEW_HPP

#include <gui_generated/victory_screen/VictoryViewBase.hpp>
#include <gui/victory_screen/VictoryPresenter.hpp>

class VictoryView : public VictoryViewBase
{
public:
    VictoryView();
    virtual ~VictoryView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // VICTORYVIEW_HPP
