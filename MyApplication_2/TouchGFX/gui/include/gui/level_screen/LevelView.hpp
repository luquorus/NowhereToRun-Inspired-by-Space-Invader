#ifndef LEVELVIEW_HPP
#define LEVELVIEW_HPP

#include <gui_generated/level_screen/LevelViewBase.hpp>
#include <gui/level_screen/LevelPresenter.hpp>

class LevelView : public LevelViewBase
{
public:
    LevelView();
    virtual ~LevelView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // LEVELVIEW_HPP
