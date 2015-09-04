#include "StandardDisplacementPolicy.hpp"


void StandardDisplacementPolicy::getRelatedTimeNodes(
        ScenarioModel& scenario,
        const Id<TimeNodeModel>& firstTimeNodeMovedId,
        QVector<Id<TimeNodeModel> >& translatedTimeNodes)
{
    if (*firstTimeNodeMovedId.val() == 0 || *firstTimeNodeMovedId.val() == 1 )
        return;

    if(translatedTimeNodes.indexOf(firstTimeNodeMovedId) == -1)
    {
        translatedTimeNodes.push_back(firstTimeNodeMovedId);
    }
    else // timeNode already moved
    {
        return;
    }

    const auto& cur_timeNode = scenario.timeNode(firstTimeNodeMovedId);
    for(const auto& cur_eventId : cur_timeNode.events())
    {
        const auto& cur_event = scenario.event(cur_eventId);

        for(const auto& state_id : cur_event.states())
        {
            auto& state = scenario.state(state_id);
            if(state.nextConstraint())
            {
                auto cons = state.nextConstraint();
                auto endStateId = scenario.constraint(cons).endState();
                auto endTnId = scenario.event(scenario.state(endStateId).eventId()).timeNode();
                getRelatedTimeNodes(scenario, endTnId, translatedTimeNodes);
            }
        }
    }
}

//----------------------------------------------------------------------------------------------


void
GoodOldDisplacementPolicy::computeDisplacement(
        ScenarioModel& scenario,
        const QVector<Id<TimeNodeModel>>& draggedElements,
        const TimeValue& deltaTime,
        ElementsProperties& elementsProperties)
{
    // this old behavior supports only the move of one timenode
    if(draggedElements.length() != 1)
    {
        //TODO: log something?
        // move nothing, nothing to undo or redo
        return;
    }else
    {
        const Id<TimeNodeModel>& firstTimeNodeMovedId = draggedElements.at(0);
        QVector<Id<TimeNodeModel> > timeNodesToTranslate;

        StandardDisplacementPolicy::getRelatedTimeNodes(scenario, firstTimeNodeMovedId, timeNodesToTranslate);

        // put each concerned timenode in modified elements
        for(auto& curTimeNodeId : timeNodesToTranslate)
        {
            auto& curTimeNode = scenario.timeNode(curTimeNodeId);
            TimenodeProperties curTimeNodeProperties;

            curTimeNodeProperties.id = curTimeNode.id();
            curTimeNodeProperties.oldDate = curTimeNode.date();
            curTimeNodeProperties.oldDate = curTimeNode.date() + deltaTime;

            elementsProperties.timenodes.push_back(curTimeNodeProperties);
        }

        // put every constraint in the scenario to be updated like old behavior
        for(auto& curConstraint : scenario.constraints)
        {
            ConstraintProperties curConstraintProperties;

            curConstraintProperties.id = curConstraint.id();
            // TODO: when min and max comme again on the table
//            curConstraintProperties.oldMin =
//            curConstraintProperties.oldMax =
//            curConstraintProperties.newMin =
//            curConstraintProperties.newMax =

            elementsProperties.constraints.push_back(curConstraintProperties);
        }

    }
}
