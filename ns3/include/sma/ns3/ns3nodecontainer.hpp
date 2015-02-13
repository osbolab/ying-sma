#pragma once

#include <sma/linklayerimpl.hpp>
#include <sma/prforwardstrategy.hpp>

#include <sma/component.hpp>

#include <sma/ccn/ccnnode.hpp>
#include <sma/neighborhelperimpl.hpp>
#include <sma/ccn/interesthelperimpl.hpp>
#include <sma/ccn/contenthelperimpl.hpp>
#include <sma/ccn/behaviorhelperimpl.hpp>

#include <sma/schedule/forwardschedulerimpl.hpp>

#include <sma/ns3/action.hpp>

#include <ns3/application.h>

#include <cstdint>
#include <memory>
#include <vector>


namespace sma
{

class Ns3NodeContainer final : public ns3::Application
{
public:
  // ns3 constructs our object while injecting config parameters into it.
  // We use this to look up the class when giving ns3 applications.
  static ns3::TypeId TypeId();

  Ns3NodeContainer();

  Ns3NodeContainer(Ns3NodeContainer const&) = delete;
  Ns3NodeContainer& operator=(Ns3NodeContainer const&) = delete;

  ~Ns3NodeContainer();

  void add_component(std::unique_ptr<Component> c);

  /*! \brief Construct an action of type A with the given arguments and sort it
   *          into the existing actions in reverse-chronological order
   *          (most to least distant in the future).
   *          If this action occurs farther in the future than all others then
   *          this is O(1).
   */
  template <typename A, typename Duration, typename... Args>
  void act_emplace_back(Duration const& delay, Args&&... args);
  /*! \brief Construct an action of type A with the given arguments and sort it
   *          into the existing actions in chronological order.
   *          If this action occurs sooner than all others then this is O(1).
   */
  template <typename A, typename Duration, typename... Args>
  void act_emplace_front(Duration const& delay, Args&&... args);

  /*! \brief Schedule the next action to run at the given time. */
  void act_schedule(Action& act);
  /*! \brief Dequeue the next action, run it (potentially enqueing additional
   *          actions), and schedule the next soonest (potentially new) action.
   */
  void act_next();

  // Hackity hack
  std::unique_ptr<CcnNode> node;

protected:
  virtual void DoDispose() override;
  virtual void StartApplication() override;
  virtual void StopApplication() override;

private:
  template <typename A, typename... Args>
  Action* make_action(Args&&... args);

  std::uint16_t prop_id;

  std::unique_ptr<Context> ctx;
  std::unique_ptr<LinkLayerImpl> linklayer;
  std::unique_ptr<PrForwardStrategy> fwd_strat;

  std::unique_ptr<NeighborHelperImpl> neighbor_helper;
  std::unique_ptr<InterestHelperImpl> interest_helper;
  std::unique_ptr<BehaviorHelperImpl> behavior_helper;
  std::unique_ptr<ForwardSchedulerImpl> scheduler_helper;

  std::unique_ptr<ContentHelperImpl> content_helper;

  std::vector<std::unique_ptr<Component>> components;

  std::deque<std::unique_ptr<Action>> actions;
};


template <typename A, typename... Args>
Action* Ns3NodeContainer::make_action(Args&&... args)
{
  return static_cast<Action*>(new A(*this, std::forward<Args>(args)...));
}

template <typename A, typename Duration, typename... Args>
void Ns3NodeContainer::act_emplace_front(Duration const& delay, Args&&... args)
{
  auto act = make_action<A>(std::forward<Args>(args)...);
  act->time = Action::delay_to_absolute(delay);

  if (!actions.empty())
    for (auto it = actions.begin(); it != actions.end(); ++it)
      if ((*it)->time >= act->time) {
        actions.emplace(it, act);
        return;
      }

  actions.emplace_back(act);
}

template <typename A, typename Duration, typename... Args>
void Ns3NodeContainer::act_emplace_back(Duration const& delay, Args&&... args)
{
  auto act = make_action<A>(std::forward<Args>(args)...);
  act->time = Action::delay_to_absolute(delay);

  if (!actions.empty())
    for (auto it = actions.rbegin(); it != actions.rend(); ++it)
      if ((*it)->time < act->time) {
        actions.emplace(it.base(), act);
        return;
      }

  actions.emplace_front(act);
}
}
