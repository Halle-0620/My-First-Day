#include "samplestory.h"

#include "gamestate.h"

namespace {

NarrativeInteraction makeInteraction(const QString &id,
                                     const QString &text,
                                     InteractionItemType type,
                                     const QString &nextSceneId = QString())
{
    NarrativeInteraction interaction;
    interaction.id = id;
    interaction.text = text;
    interaction.type = type;
    interaction.nextSceneId = nextSceneId;
    return interaction;
}

} // namespace

NarrativeSceneMap createSampleStory()
{
    NarrativeSceneMap scenes;

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("intro_narration");
        scene.header = QStringLiteral("引擎测试 / 普通推进");
        scene.displayMode = NarrativeDisplayMode::Narration;
        scene.backgroundStyle = BackgroundStyle::SoftNarration;
        scene.text = QStringLiteral("这是阶段 2 的最小叙事引擎测试。现在开始的推进，不再由 MockFlowController 驱动。");
        scene.nextSceneId = QStringLiteral("intro_dialogue");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("intro_dialogue");
        scene.header = QStringLiteral("引擎测试 / 普通推进");
        scene.displayMode = NarrativeDisplayMode::Dialogue;
        scene.backgroundStyle = BackgroundStyle::QuietHallway;
        scene.speaker = QStringLiteral("灵灵");
        scene.text = QStringLiteral("这一句用来确认普通对白也已经接入了引擎。继续之后，会进入真正的交互节点。");
        scene.nextSceneId = QStringLiteral("choice_downstairs");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("choice_downstairs");
        scene.header = QStringLiteral("引擎测试 / 普通选项");
        scene.displayMode = NarrativeDisplayMode::Interaction;
        scene.backgroundStyle = BackgroundStyle::ClassroomDusk;
        scene.speaker = QStringLiteral("旁白");
        scene.text = QStringLiteral("这里是一个普通选项节点。选择不同项会进入不同后续文本，但之后会重新汇回同一路径。");
        scene.interactionMode = InteractionMode::Choice;

        NarrativeInteraction goDown = makeInteraction(
            QStringLiteral("go_downstairs"),
            QStringLiteral("跟他下楼走两圈"),
            InteractionItemType::Choice,
            QStringLiteral("choice_feedback_down")
        );
        goDown.boolWrites.insert(QStringLiteral("went_downstairs"), true);

        NarrativeInteraction stayUp = makeInteraction(
            QStringLiteral("stay_classroom"),
            QStringLiteral("想在教室里写作业"),
            InteractionItemType::Choice,
            QStringLiteral("choice_feedback_stay")
        );
        stayUp.boolWrites.insert(QStringLiteral("went_downstairs"), false);

        scene.interactions = {goDown, stayUp};
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("choice_feedback_down");
        scene.header = QStringLiteral("引擎测试 / 选项反馈");
        scene.displayMode = NarrativeDisplayMode::Narration;
        scene.backgroundStyle = BackgroundStyle::QuietHallway;
        scene.text = QStringLiteral("你选了“下楼”。这里是一条局部反馈文本，随后会回到统一流程。");
        scene.nextSceneId = QStringLiteral("location_pick");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("choice_feedback_stay");
        scene.header = QStringLiteral("引擎测试 / 选项反馈");
        scene.displayMode = NarrativeDisplayMode::Narration;
        scene.backgroundStyle = BackgroundStyle::ClassroomDusk;
        scene.text = QStringLiteral("你选了“留在教室”。这同样只是局部反馈，后面仍然会汇回统一流程。");
        scene.nextSceneId = QStringLiteral("location_pick");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("location_pick");
        scene.header = QStringLiteral("引擎测试 / 热点选择");
        scene.displayMode = NarrativeDisplayMode::Interaction;
        scene.backgroundStyle = BackgroundStyle::ChoiceFocus;
        scene.speaker = QStringLiteral("旁白");
        scene.text = QStringLiteral("这里是热点型交互节点。点击地点后，会写入字符串变量 `hide_place`，再进入统一后续节点。");
        scene.interactionMode = InteractionMode::Hotspot;

        NarrativeInteraction tree = makeInteraction(
            QStringLiteral("tree_corner"),
            QStringLiteral("去树下"),
            InteractionItemType::Hotspot,
            QStringLiteral("location_feedback")
        );
        tree.stringWrites.insert(QStringLiteral("hide_place"), QStringLiteral("tree"));

        NarrativeInteraction gym = makeInteraction(
            QStringLiteral("gym_back"),
            QStringLiteral("去体育馆后面"),
            InteractionItemType::Hotspot,
            QStringLiteral("location_feedback")
        );
        gym.stringWrites.insert(QStringLiteral("hide_place"), QStringLiteral("gym_back"));

        scene.interactions = {tree, gym};
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("location_feedback");
        scene.header = QStringLiteral("引擎测试 / 热点反馈");
        scene.displayMode = NarrativeDisplayMode::Narration;
        scene.backgroundResolver = [](const GameState &state) {
            return state.stringValue(QStringLiteral("hide_place")) == QStringLiteral("tree")
                ? BackgroundStyle::SoftNarration
                : BackgroundStyle::NightRain;
        };
        scene.textResolver = [](const GameState &state) {
            const QString place = state.stringValue(QStringLiteral("hide_place"));
            if (place == QStringLiteral("tree")) {
                return QStringLiteral("你选了树下。背景样式也跟着切成了更像临时避难所的感觉。");
            }

            return QStringLiteral("你选了体育馆后面。这里读取了地点变量，并给出另一种环境反馈。");
        };
        scene.nextSceneId = QStringLiteral("topic_confession");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("topic_confession");
        scene.header = QStringLiteral("引擎测试 / 逐个点击完成");
        scene.displayMode = NarrativeDisplayMode::Interaction;
        scene.backgroundStyle = BackgroundStyle::QuietHallway;
        scene.speaker = QStringLiteral("旁白");
        scene.text = QStringLiteral("这里是 topic 型节点。四个项目都要点完，继续按钮才会出现。");
        scene.interactionMode = InteractionMode::Topic;
        scene.requiresAllInteractions = true;
        scene.completionNextSceneId = QStringLiteral("action_prompt");
        scene.completionTextResolver = [](const GameState &state) {
            const QString lastTopic = state.stringValue(QStringLiteral("last_confession_topic"));
            return QStringLiteral("四个话题都已经点完了。当前记录的最后点击话题是：%1。现在可以继续推进。").arg(lastTopic);
        };

        NarrativeInteraction people = makeInteraction(
            QStringLiteral("topic_people"),
            QStringLiteral("人际"),
            InteractionItemType::Topic
        );
        people.feedbackText = QStringLiteral("你点开了“人际”。引擎会保留当前节点，并把这个项目标记为已访问。");
        people.stringWrites.insert(QStringLiteral("last_confession_topic"), QStringLiteral("人际"));

        NarrativeInteraction discipline = makeInteraction(
            QStringLiteral("topic_discipline"),
            QStringLiteral("处分"),
            InteractionItemType::Topic
        );
        discipline.feedbackText = QStringLiteral("你点开了“处分”。在所有话题点完前，这一类节点不会放行继续按钮。");
        discipline.stringWrites.insert(QStringLiteral("last_confession_topic"), QStringLiteral("处分"));

        NarrativeInteraction home = makeInteraction(
            QStringLiteral("topic_home"),
            QStringLiteral("家里"),
            InteractionItemType::Topic
        );
        home.feedbackText = QStringLiteral("你点开了“家里”。这一步验证的是“逐个点击完成”而不是多分支跳转。");
        home.stringWrites.insert(QStringLiteral("last_confession_topic"), QStringLiteral("家里"));

        NarrativeInteraction body = makeInteraction(
            QStringLiteral("topic_body"),
            QStringLiteral("身体"),
            InteractionItemType::Topic
        );
        body.feedbackText = QStringLiteral("你点开了“身体”。最后点击的内容会被写进字符串变量，供后续节点读取。");
        body.stringWrites.insert(QStringLiteral("last_confession_topic"), QStringLiteral("身体"));

        scene.interactions = {people, discipline, home, body};
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("action_prompt");
        scene.header = QStringLiteral("引擎测试 / 行为交互");
        scene.displayMode = NarrativeDisplayMode::Interaction;
        scene.backgroundStyle = BackgroundStyle::ClassroomDusk;
        scene.speaker = QStringLiteral("旁白");
        scene.text = QStringLiteral("这里是 action 型节点。“开口 / 沉默”会写入变量并进入同一个后续场景。");
        scene.interactionMode = InteractionMode::Action;

        NarrativeInteraction speak = makeInteraction(
            QStringLiteral("action_speak"),
            QStringLiteral("开口"),
            InteractionItemType::Action,
            QStringLiteral("action_feedback")
        );
        speak.stringWrites.insert(QStringLiteral("office_action"), QStringLiteral("开口"));

        NarrativeInteraction silent = makeInteraction(
            QStringLiteral("action_silent"),
            QStringLiteral("沉默"),
            InteractionItemType::Action,
            QStringLiteral("action_feedback")
        );
        silent.stringWrites.insert(QStringLiteral("office_action"), QStringLiteral("沉默"));

        scene.interactions = {speak, silent};
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("action_feedback");
        scene.header = QStringLiteral("引擎测试 / 行为反馈");
        scene.displayMode = NarrativeDisplayMode::Dialogue;
        scene.backgroundStyle = BackgroundStyle::ClassroomDusk;
        scene.speakerResolver = [](const GameState &) {
            return QStringLiteral("川哥");
        };
        scene.textResolver = [](const GameState &state) {
            const QString action = state.stringValue(QStringLiteral("office_action"));
            return QStringLiteral("你刚才选择了“%1”。这里读取了行为变量，但不让它改变大走向，接下来仍然会汇到同一段演出。").arg(action);
        };
        scene.nextSceneId = QStringLiteral("performance_beat");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("performance_beat");
        scene.header = QStringLiteral("引擎测试 / 纯演出段");
        scene.displayMode = NarrativeDisplayMode::Performance;
        scene.backgroundStyle = BackgroundStyle::EndingGlow;
        scene.text = QStringLiteral("停一下。\n\n只剩下呼吸声。\n\n让这一段自己落下去。");
        scene.nextSceneId = QStringLiteral("summary");
        scenes.insert(scene.id, scene);
    }

    {
        NarrativeScene scene;
        scene.id = QStringLiteral("summary");
        scene.header = QStringLiteral("引擎测试 / 变量承接");
        scene.displayMode = NarrativeDisplayMode::Narration;
        scene.backgroundStyle = BackgroundStyle::EndingGlow;
        scene.textResolver = [](const GameState &state) {
            const QString wentDownstairs = state.boolValue(QStringLiteral("went_downstairs")) ? QStringLiteral("是") : QStringLiteral("否");
            const QString hidePlace = state.stringValue(QStringLiteral("hide_place"), QStringLiteral("未选择"));
            const QString lastTopic = state.stringValue(QStringLiteral("last_confession_topic"), QStringLiteral("未记录"));

            return QStringLiteral(
                "演示结束。\n\n"
                "当前变量读取结果：\n"
                "went_downstairs = %1\n"
                "hide_place = %2\n"
                "last_confession_topic = %3\n\n"
                "这说明最小叙事骨架已经能把前面节点写入的变量承接到后续文本里。")
                .arg(wentDownstairs, hidePlace, lastTopic);
        };
        scenes.insert(scene.id, scene);
    }

    return scenes;
}

QString sampleStoryStartSceneId()
{
    return QStringLiteral("intro_narration");
}
