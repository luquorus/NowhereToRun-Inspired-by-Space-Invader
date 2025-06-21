#ifndef VICTORYPRESENTER_HPP
#define VICTORYPRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class VictoryView;

class VictoryPresenter : public touchgfx::Presenter, public ModelListener
{
public:
    VictoryPresenter(VictoryView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~VictoryPresenter() {}

private:
    VictoryPresenter();

    VictoryView& view;
};

#endif // VICTORYPRESENTER_HPP
