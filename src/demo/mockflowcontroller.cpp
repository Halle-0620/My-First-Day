#include "mockflowcontroller.h"

#include <QtGlobal>

namespace {

InteractionItem makeItem(const QString &id,
                         const QString &text,
                         InteractionItemType type,
                         bool enabled = true,
                         bool visited = false)
{
    return {id, text, type, enabled, visited, false};
}

} // namespace

MockFlowController::MockFlowController()
    : m_currentStep(Step::IntroNarration)
{
}

DemoUiState MockFlowController::currentState() const
{
    return stateForCurrentStep();
}

DemoUiState MockFlowController::advance()
{
    switch (m_currentStep) {
    case Step::IntroNarration:
        m_currentStep = Step::StandardDialogue;
        break;
    case Step::StandardDialogue:
        m_currentStep = Step::ChoicePrompt;
        break;
    case Step::ChoicePrompt:
        break;
    case Step::ChoiceFeedback:
        m_currentStep = Step::LocationPrompt;
        break;
    case Step::LocationPrompt:
        break;
    case Step::LocationFeedback:
        m_currentStep = Step::TopicPrompt;
        break;
    case Step::TopicPrompt:
        if (allTopicsVisited()) {
            m_currentStep = Step::ActionPrompt;
        }
        break;
    case Step::ActionPrompt:
        break;
    case Step::ActionFeedback:
        m_currentStep = Step::PerformanceBeat;
        break;
    case Step::PerformanceBeat:
        m_currentStep = Step::ObjectPrompt;
        break;
    case Step::ObjectPrompt:
        if (allObjectsVisited()) {
            m_currentStep = Step::EndCard;
        }
        break;
    case Step::EndCard:
        break;
    }

    return stateForCurrentStep();
}

DemoUiState MockFlowController::triggerInteraction(const QString &id)
{
    if (m_currentStep == Step::ChoicePrompt) {
        m_choiceResultId = id;
        m_currentStep = Step::ChoiceFeedback;
    } else if (m_currentStep == Step::LocationPrompt) {
        m_selectedLocationId = id;
        m_currentStep = Step::LocationFeedback;
    } else if (m_currentStep == Step::TopicPrompt && !m_visitedTopics.contains(id)) {
        m_lastTopicId = id;
        m_visitedTopics.insert(id);
    } else if (m_currentStep == Step::ActionPrompt) {
        m_selectedActionId = id;
        m_currentStep = Step::ActionFeedback;
    } else if (m_currentStep == Step::ObjectPrompt && !m_visitedObjects.contains(id)) {
        m_lastObjectId = id;
        m_visitedObjects.insert(id);
    }

    return stateForCurrentStep();
}

DemoUiState MockFlowController::stateForCurrentStep() const
{
    switch (m_currentStep) {
    case Step::IntroNarration:
        return {
            QStringLiteral("UI 交互演示 / 开场"),
            QString(),
            QStringLiteral("这是阶段 1 收尾用的交互层预演。当前先展示纯旁白推进状态，名字框隐藏，继续按钮可用。"),
            BackgroundWidget::BackgroundStyle::SoftNarration,
            true,
            false,
            true,
            InteractionMode::None,
            {}
        };
    case Step::StandardDialogue:
        return {
            QStringLiteral("UI 交互演示 / 对白"),
            QStringLiteral("灵灵"),
            QStringLiteral("这里先保留一段普通对白，确认通用交互层补完之后，常规对白推进仍然稳定。"),
            BackgroundWidget::BackgroundStyle::QuietHallway,
            true,
            true,
            true,
            InteractionMode::None,
            {}
        };
    case Step::ChoicePrompt:
        return {
            QStringLiteral("演示 0 / 普通选项"),
            QStringLiteral("旁白"),
            QStringLiteral("先看普通选项模式。这里仍然是标准的二选一，只是它现在也走统一交互区。"),
            BackgroundWidget::BackgroundStyle::ClassroomDusk,
            true,
            true,
            false,
            InteractionMode::Choice,
            {
                makeItem(QStringLiteral("choice_step_closer"), QStringLiteral("先看靠近一点的反馈"), InteractionItemType::Choice),
                makeItem(QStringLiteral("choice_hold_back"), QStringLiteral("先看再等等的反馈"), InteractionItemType::Choice)
            }
        };
    case Step::ChoiceFeedback:
        return {
            QStringLiteral("演示 0 / 选项反馈"),
            QString(),
            m_choiceResultId == QStringLiteral("choice_step_closer")
                ? QStringLiteral("你点了第一个普通选项。交互区已经收起，界面回到反馈文本，再继续推进。")
                : QStringLiteral("你点了第二个普通选项。这里同样只做局部反馈，不进入真正分线。"),
            BackgroundWidget::BackgroundStyle::ClassroomDusk,
            true,
            false,
            true,
            InteractionMode::None,
            {}
        };
    case Step::LocationPrompt:
        return {
            QStringLiteral("演示 1 / 地点热点"),
            QStringLiteral("旁白"),
            QStringLiteral("这里演示地点型热点。点击任意地点后，会先更新反馈文本，再切到对应背景风格。"),
            BackgroundWidget::BackgroundStyle::ChoiceFocus,
            true,
            true,
            false,
            InteractionMode::Hotspot,
            {
                makeItem(QStringLiteral("place_tree"), QStringLiteral("去树下"), InteractionItemType::Hotspot),
                makeItem(QStringLiteral("place_gym"), QStringLiteral("去体育馆后面"), InteractionItemType::Hotspot)
            }
        };
    case Step::LocationFeedback:
        return {
            QStringLiteral("演示 1 / 地点反馈"),
            QString(),
            m_selectedLocationId == QStringLiteral("place_tree")
                ? QStringLiteral("你点了“去树下”。背景切成了更像临时避难所的风格，文本也先给出一小段反馈。")
                : QStringLiteral("你点了“去体育馆后面”。背景切成了更安静封闭的风格，说明热点点击和背景反馈已经连起来了。"),
            m_selectedLocationId == QStringLiteral("place_tree")
                ? BackgroundWidget::BackgroundStyle::SoftNarration
                : BackgroundWidget::BackgroundStyle::NightRain,
            true,
            false,
            true,
            InteractionMode::None,
            {}
        };
    case Step::TopicPrompt:
        return {
            QStringLiteral("演示 2 / 逐个点击话题"),
            QStringLiteral("旁白"),
            allTopicsVisited()
                ? QStringLiteral("四个话题都已经点完了。继续按钮现在才会出现，并且最后点击的话题已经在 demo 内部被记录下来。")
                : (m_lastTopicId.isEmpty()
                       ? QStringLiteral("这里演示逐个点击完成的交互。四个话题都要点过，继续按钮才会出现。")
                       : topicFeedbackText(m_lastTopicId)),
            BackgroundWidget::BackgroundStyle::QuietHallway,
            true,
            true,
            allTopicsVisited(),
            InteractionMode::Topic,
            buildTopicItems()
        };
    case Step::ActionPrompt:
        return {
            QStringLiteral("演示 3 / 行为交互"),
            QStringLiteral("旁白"),
            QStringLiteral("这里演示行为式交互。它更像动作触发，而不是标准分支题，但点击后会先给局部反馈。"),
            BackgroundWidget::BackgroundStyle::ClassroomDusk,
            true,
            true,
            false,
            InteractionMode::Action,
            {
                makeItem(QStringLiteral("action_enter"), QStringLiteral("进去"), InteractionItemType::Action),
                makeItem(QStringLiteral("action_silent"), QStringLiteral("沉默"), InteractionItemType::Action)
            }
        };
    case Step::ActionFeedback:
        return {
            QStringLiteral("演示 3 / 行为反馈"),
            QStringLiteral("旁白"),
            m_selectedActionId == QStringLiteral("action_enter")
                ? QStringLiteral("你点了“进去”。这类行为交互会先给出一小段动作反馈，但后续仍然汇到同一个步骤。")
                : QStringLiteral("你点了“沉默”。这里保留了局部变化，但没有真正分出一条新路线。"),
            BackgroundWidget::BackgroundStyle::ClassroomDusk,
            true,
            true,
            true,
            InteractionMode::None,
            {}
        };
    case Step::PerformanceBeat:
        return {
            QStringLiteral("演示 4 / 纯演出段"),
            QString(),
            QStringLiteral("停一下。\n\n只留文本。\n\n让这一拍自己落下去。"),
            BackgroundWidget::BackgroundStyle::EndingGlow,
            true,
            false,
            true,
            InteractionMode::None,
            {}
        };
    case Step::ObjectPrompt:
        return {
            QStringLiteral("演示 5 / 可点击对象"),
            QString(),
            allObjectsVisited()
                ? QStringLiteral("这些对象都已经点过了。继续按钮现在出现，说明“无传统选项框”的可点击对象交互也能承载。")
                : (m_lastObjectId.isEmpty()
                       ? QStringLiteral("这里演示可点击对象风格。它不像传统选项题，更像画面里几个短词可以被逐个触发。")
                       : objectFeedbackText(m_lastObjectId)),
            BackgroundWidget::BackgroundStyle::NightRain,
            true,
            false,
            allObjectsVisited(),
            InteractionMode::Object,
            buildObjectItems()
        };
    case Step::EndCard:
        return {
            QStringLiteral("UI 交互演示 / 结束"),
            QString(),
            QStringLiteral("演示结束。当前壳子已经能承载普通选项、地点热点、行为项、逐个点击话题、纯演出段和对象点击这几类基础交互。"),
            BackgroundWidget::BackgroundStyle::EndingGlow,
            true,
            false,
            false,
            InteractionMode::None,
            {}
        };
    }

    return {};
}

bool MockFlowController::allTopicsVisited() const
{
    return m_visitedTopics.size() == 4;
}

bool MockFlowController::allObjectsVisited() const
{
    return m_visitedObjects.size() == 3;
}

QString MockFlowController::topicFeedbackText(const QString &id) const
{
    if (id == QStringLiteral("topic_people")) {
        return QStringLiteral("你先点开了“人际”。这一步只用占位文本验证：点击后会即时反馈，同时把项目标记成已访问。");
    }

    if (id == QStringLiteral("topic_discipline")) {
        return QStringLiteral("你点开了“处分”。按钮现在会变成已访问状态，而且在四项点完前，继续按钮不会提前出现。");
    }

    if (id == QStringLiteral("topic_home")) {
        return QStringLiteral("你点开了“家里”。这说明通用交互层已经能支持“逐个点完再继续”的结构。");
    }

    return QStringLiteral("你点开了“身体”。最后点击的是哪一项，demo 会在内部记住，但还没有进入正式变量系统。");
}

QString MockFlowController::objectFeedbackText(const QString &id) const
{
    if (id == QStringLiteral("object_rain")) {
        return QStringLiteral("你点了“雨”。对象式交互会给很短的反馈，更像摸到画面里的一个点。");
    }

    return QStringLiteral("你点了“路灯”。已点过的对象会变灰，直到这一组都点完。");
}

InteractionItems MockFlowController::buildTopicItems() const
{
    return {
        makeItem(QStringLiteral("topic_people"),
                 QStringLiteral("人际"),
                 InteractionItemType::Topic,
                 !m_visitedTopics.contains(QStringLiteral("topic_people")),
                 m_visitedTopics.contains(QStringLiteral("topic_people"))),
        makeItem(QStringLiteral("topic_discipline"),
                 QStringLiteral("处分"),
                 InteractionItemType::Topic,
                 !m_visitedTopics.contains(QStringLiteral("topic_discipline")),
                 m_visitedTopics.contains(QStringLiteral("topic_discipline"))),
        makeItem(QStringLiteral("topic_home"),
                 QStringLiteral("家里"),
                 InteractionItemType::Topic,
                 !m_visitedTopics.contains(QStringLiteral("topic_home")),
                 m_visitedTopics.contains(QStringLiteral("topic_home"))),
        makeItem(QStringLiteral("topic_body"),
                 QStringLiteral("身体"),
                 InteractionItemType::Topic,
                 !m_visitedTopics.contains(QStringLiteral("topic_body")),
                 m_visitedTopics.contains(QStringLiteral("topic_body")))
    };
}

InteractionItems MockFlowController::buildObjectItems() const
{
    return {
        makeItem(QStringLiteral("object_rain"),
                 QStringLiteral("雨"),
                 InteractionItemType::Object,
                 !m_visitedObjects.contains(QStringLiteral("object_rain")),
                 m_visitedObjects.contains(QStringLiteral("object_rain"))),
        makeItem(QStringLiteral("object_light"),
                 QStringLiteral("路灯"),
                 InteractionItemType::Object,
                 !m_visitedObjects.contains(QStringLiteral("object_light")),
                 m_visitedObjects.contains(QStringLiteral("object_light")))
    };
}
