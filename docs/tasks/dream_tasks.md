# Dream Tasks

## 总原则

梦境是 7B 支线隐藏彩蛋，不是主线，不是新结局。
梦境结束后必须进入场景10。

执行方式：
Codex 每次只执行 dream_status.md 中第一个 todo STEP。
不要一次性执行所有 STEP。

## STEP 索引

- STEP0_AUDIT_CURRENT_DREAM：检查当前梦境实现，不修改
- STEP1_CLEANUP_DREAM：清理旧梦境实现
- STEP2_PROJECT_STRUCTURE_ANALYSIS：分析工程结构
- STEP3_IMPLEMENTATION_PLAN：写实现计划
- STEP4_DREAM_SCENE_SEQUENCE：实现普通 scene 序列
- STEP5_TRIGGER_CONDITION：接入触发条件
- STEP6_BASIC_PRESENTATION：基础演出
- STEP7_OPTIONAL_SHADER：可选 shader
- STEP8_DREAM_AUDIO：音效
- STEP9_QA：QA


## STEP1_CLEANUP_DREAM

请只做清理，不要重新实现梦境。

执行前必须参考 STEP0_AUDIT_CURRENT_DREAM 的检查结果。

目标：
把梦境相关实现恢复到一个可控状态。

要求：
1. 保留原有主线和 7B 支线逻辑。
2. 不改变场景10、11、12。
3. 移除明显错误、重复、无用的梦境代码。
4. 如果某些梦境代码不确定是否有用，请先注释并说明，不要直接删除。
5. 清理后保证游戏仍能正常编译运行。
6. 本轮不要新增梦境内容。
7. 不要加 shader。
8. 不要加音效。
9. 不要重构 SceneManager。
10. 不要修改剧情文本。

完成后：
1. 将 dream_status.md 中 STEP1_CLEANUP_DREAM 标记为 done。
2. 汇报：
   - 删除/注释了什么
   - 保留了什么
   - 当前梦境入口是否还存在
   - 如何测试 7B 支线没有坏

DREAM_1 下坠与破碎：
- 黑屏
- 出现 shader/抽象画面
- 文本短句：
  先是往下掉。
  试卷、灯光、数字，碎成很多片。
  有人在说话。
  但你听不清。

DREAM_2 悬停：
- 画面从混乱转为安静
- 文本：
  然后，忽然安静了。
  你没有继续往下掉。
  像是落进了一团很轻的风里。

DREAM_3 未来碎片：
依次出现三个短碎片：
A. 房间与风：
  后来你有了自己的桌子。
  有风会吹进来。
  早晨不再总是这么紧。
B. 路与远方：
  后来你会去很多地方。
  路很长。
  但不再只通向一间教室。
C. 创作：
  后来你会把一些说不出口的东西，
  慢慢做成别人也能看见的东西。

DREAM_4 收束：
- 文本：
  你没有梦见答案。
  只是梦见，天会亮。

DREAM_5 醒来：
- shader 淡出
- 黑屏淡出
- 下课铃渐入
- 教室环境声回来
- 文本：
  铃声响起来的时候，你慢慢睁开眼。
  世界还在原处。
  可你没有刚才那么紧了。
- 设置 side_route_mode = "dream"
- 跳转到场景10

请输出：
1. 需要新增哪些 scene ID
2. 需要新增/复用哪些变量
3. 每个 scene 的进入条件
4. 每个 scene 的下一个跳转
5. 需要哪些演出命令
6. 哪些地方先用占位实现
7. 最小代码改动方案

本轮只写计划，不写代码。

STEP3：
请根据上一轮确定的梦境设计，只实现“梦境场景数据和跳转”，不要做 shader、音效、复杂转场。

要求：
1. 新增梦境 scene 序列：
   dream_0_sleep
   dream_1_fall
   dream_2_suspend
   dream_3_future_room
   dream_3_future_road
   dream_3_future_create
   dream_4_end
   dream_5_wake
2. 每个 dream scene 使用普通文本推进。
3. 先使用现有背景或黑屏占位，不新增复杂视觉效果。
4. dream_5_wake 结束后跳转到 scene10。
5. 设置 side_route_mode = "dream"。
6. 不改主线 7A、场景8、场景10、场景11、场景12。
7. 不改现有 7B 写作业路线。
8. 不新增 shader。
9. 不新增音效。
10. 不修改 AudioManager。
11. 不改变已有变量名，除非当前项目没有这些变量。

梦境文本必须使用以下内容，不要自行扩写：

dream_0_sleep:
你本来只是想闭一会儿眼。
可不知不觉，意识真的慢慢沉下去了。

dream_1_fall:
先是往下掉。
试卷、灯光、数字，碎成很多片。
有一阵很远的声音。
但你听不清。

dream_2_suspend:
然后，忽然安静了。
你没有继续往下掉。
像是落进了一团很轻的风里。

dream_3_future_room:
后来你有了自己的桌子。
有风会吹进来。
早晨不再总是这么紧。

dream_3_future_road:
后来你会去很多地方。
路很长。
但不再只通向一间教室。

dream_3_future_create:
后来你会把一些说不出口的东西，
慢慢做成别人也能看见的东西。

dream_4_end:
你没有梦见答案。
只是梦见，天会亮。

dream_5_wake:
铃声响起来的时候，你慢慢睁开眼。
世界还在原处。
可你没有刚才那么紧了。

完成后汇报：
1. 修改文件
2. 新增 scene ID
3. dream_5_wake 如何跳转到 scene10
4. 如何测试直接进入梦境
5. 如何测试梦境结束后进入 scene10

STEP4：
请接入梦境触发条件，只修改 7B 中“睡一觉”的分支判断。

规则：
1. 如果 rest_flag_scene1 == true 且 rest_flag_scene5 == true，并且玩家在 7B 选择“睡一觉”，进入 dream_0_sleep。
2. 如果条件不满足，走原本的“睡不着”文本。
3. 进入梦境时设置 side_route_mode = "dream"。
4. 未进入梦境时，如果是睡不着，设置 side_route_mode = "sleep_failed" 或沿用项目已有值。
5. 不改变“自己写作业”路线。
6. 不改变主线 7A。
7. 不改变场景10之后的流程。
8. 不新增音效。
9. 不新增 shader。

完成后汇报：
1. 修改了哪个分支判断
2. rest_flag_scene1 在哪里设置
3. rest_flag_scene5 在哪里设置
4. 7B 选择睡觉时如何判断
5. 如何测试：
   - 条件满足进入梦境
   - 条件不满足睡不着
   - 写作业路线不受影响

## STEP4B_UPDATE_FUTURE_CHOICE

请将梦境未来碎片从“顺序播放”改为“三选项选择”模式。

背景：
当前梦境已经有最小 scene 序列，但未来碎片需要改成玩家在梦境空间中主动点击三个选项：
- 远方
- 并肩
- 创作

目标：
玩家进入 dream_3_future_choice 后，看到三个选项。每点一个选项，进入对应未来碎片文本。读完后返回 dream_3_future_choice。三个选项全部读完后，进入 dream_4_end。

需要的 scene：
- dream_3_future_choice
- dream_3_future_far
- dream_3_future_together
- dream_3_future_create

如果当前已有这些旧 scene：
- dream_3_future_room
- dream_3_future_road
- dream_3_future_create

请做最小迁移：
1. `dream_3_future_road` 可以改名或映射为 `dream_3_future_far`。
2. `dream_3_future_room` 不再作为 V1 必要碎片，可移除引用或保留但不进入。
3. `dream_3_future_create` 可以继续复用。
4. 新增 `dream_3_future_together`。

文本必须使用：

dream_3_future_choice:
不显示长旁白，只显示三个选项：
- 远方
- 并肩
- 创作

dream_3_future_far:
后来你会去很多地方。
路很长。
但不再只通向一间教室。

dream_3_future_together:
后来还是会有人陪你走一段路。
不总是同一个人。
但你不会永远一个人。

dream_3_future_create:
后来你会把一些说不出口的东西，
慢慢做成别人也能看见的东西。

逻辑要求：
1. dream_2_suspend 之后进入 dream_3_future_choice。
2. dream_3_future_choice 显示三个选项。
3. 玩家点“远方”后进入 dream_3_future_far，并记录 dream_seen_far = true。
4. 玩家点“并肩”后进入 dream_3_future_together，并记录 dream_seen_together = true。
5. 玩家点“创作”后进入 dream_3_future_create，并记录 dream_seen_create = true。
6. 每个碎片读完后返回 dream_3_future_choice。
7. 已经读过的选项不再显示，或显示为已读但不可点击。
8. 三个碎片全部读完后，进入 dream_4_end。
9. 不改变 dream_0_sleep、dream_1_fall、dream_2_suspend、dream_4_end、dream_5_wake 的文本。
10. 不改主线 7A。
11. 不改场景8、10、11、12。
12. 不新增 shader。
13. 不新增音效。
14. 不修改 AudioManager。

需要新增/复用变量：
- dream_seen_far
- dream_seen_together
- dream_seen_create

完成后：
1. 将 dream_status.md 中 STEP4B_UPDATE_FUTURE_CHOICE 标记为 done。
2. 汇报：
   - 修改文件
   - 新增/修改 scene ID
   - 新增变量
   - 三个选项如何判断是否已读
   - 三个选项全部完成后如何进入 dream_4_end
   - 如何测试未来碎片选择逻辑

## STEP6_BASIC_PRESENTATION

请为梦境 scene 增加基础演出，不要接入 shader。



要求：
1. dream_0_sleep：
   - 教室背景逐渐变暗
   - 文本显示后淡出到黑屏
2. dream_1_fall：
   - 使用黑屏背景
   - 文本短句逐句出现
   - 每句之间增加轻微停顿
3. dream_2_suspend：
   - 黑屏逐渐变为较柔和的梦境占位背景，如果没有素材则继续黑屏
4. dream_3_future_*：
   - 使用同一个柔和梦境占位背景
   - 每个碎片之间淡入淡出
5. dream_4_end：
   - 保持简洁，最后淡出
6. dream_5_wake：
   - 从黑屏淡回教室背景

限制：
1. 不新增 shader。
2. 不新增音效。
3. 不改梦境文本。
4. 不改触发条件。
5. 不影响其他场景转场。
6. 如果项目已有 transition/fade 接口，请复用；没有的话，做最小实现。

完成后汇报：
1. 用了哪些已有演出接口
2. 新增了哪些演出接口
3. 是否影响其他场景
4. 如何单独测试梦境演出

完成后：
1. 将 dream_status.md 中 STEP6_BASIC_PRESENTATION 标记为 done。
2. 汇报：
   - 用了哪些已有演出接口
   - 新增了哪些演出接口
   - 是否影响其他场景
   - 如何单独测试梦境演出

## STEP7A_SHADER_INFRA

请只实现梦境 shader overlay 的基础设施，不接入具体 Shadertoy 文件。

目标：
让项目具备在梦境 scene 上方显示可选 shader overlay 的能力。

要求：
1. 新增 DreamShaderOverlay 或 DreamEffectWidget。
2. 支持启用/关闭：
   enableDreamShader = true/false
3. 支持加载 fragment shader 文件。
4. 支持 Shadertoy 风格 uniform：
   - iTime
   - iResolution
   - iChannel0
5. 如果 iChannel0 当前实现成本太高，可以先使用 fallback texture，并在报告中说明。
6. 支持 Shadertoy 风格 mainImage(out vec4 fragColor, in vec2 fragCoord) 适配。
7. shader 编译失败时 warning，并自动回退到普通背景。
8. shader overlay 不阻塞文本推进。
9. shader overlay 不遮挡文本框和选项按钮。
10. 离开 dream scene 时必须关闭并释放 shader overlay。
11. 不接入具体 dream_1_fall 或 dream_3_future_choice。
12. 不修改梦境文本。
13. 不修改梦境触发条件。
14. 不修改 AudioManager。
15. 不影响其他场景背景显示。

完成后：
1. 将 dream_status.md 中 STEP7A_SHADER_INFRA 标记为 done。
2. 汇报：
   - 新增文件
   - 如何开启/关闭 shader overlay
   - iTime/iResolution 如何传入
   - iChannel0 当前如何处理
   - Shadertoy mainImage 如何适配
   - shader 失败时如何回退
   - 如何用简单测试 shader 验证 overlay 可用


## STEP7B_DREAM_SHADER_BINDING

请把两个 dream shader 文件接入梦境 scene。

已有文件：
1. assets/shaders/dream/dream_fall_xskgrw.frag
2. assets/shaders/dream/dream_future_mtcgdf.frag

来源：
- dream_fall_xskgrw.frag: https://www.shadertoy.com/view/XsKGRW
- dream_future_mtcgdf.frag: https://www.shadertoy.com/view/MtcGDf

接入规则：
1. dream_1_fall 使用 dream_fall_xskgrw.frag。
2. dream_1_fall 的 shader 显示约 5 秒。
3. dream_1_fall 结束时关闭 fall shader。
4. dream_2_suspend 到 dream_3_future_* 使用 dream_future_mtcgdf.frag。
5. dream_3_future_choice 上显示选项大框：
   - 远方
   - 并肩
   - 创作
6. 选项按钮必须显示在 shader 上方。
7. 玩家点过的选项应隐藏或标记为已读。
8. 三个选项全部点完后进入 dream_4_end。
9. 离开梦境进入 dream_5_wake 或 scene10 前必须关闭 shader。
10. shader 编译失败时，梦境仍然可以用普通背景推进。
11. 不修改梦境文本。
12. 不修改触发条件。
13. 不影响其他场景背景显示。
14. 不修改 AudioManager。

完成后：
1. 将 dream_status.md 中 STEP7B_DREAM_SHADER_BINDING 标记为 done。
2. 汇报：
   - 每个 dream scene 对应哪个 shader
   - shader 开关位置
   - 选项如何叠加在 shader 上方
   - 三个未来碎片如何记录已读
   - shader 失败如何回退
   - 如何测试 dream_1_fall 的 5 秒效果
   - 如何测试三个未来选项全部点完后进入 dream_4_end

## STEP8_DREAM_AUDIO

请只为梦境接入音效和 BGM，不修改文本、shader、触发条件、scene 跳转。

音频设计原则：
1. 《空中散步》只从 dream_2_suspend 开始淡入，不要在 dream_0_sleep 或 dream_1_fall 播放。
2. dream_1_fall 只使用下坠/低频/碎片音，不使用 BGM。
3. dream_3_future_choice 和三个未来碎片主要依靠 BGM，不额外堆叠复杂音效。
4. dream_5_wake 用下课铃把玩家拉回现实。

需要使用的音频 ID：
- bgm_dream_air_walk
- emo_dream_fall_low
- sfx_dream_whoosh_soft
- sfx_paper_fragment
- amb_dream_air
- sfx_bell
- amb_classroom_muffled 或 amb_classroom_evening_and_hallway_loop

触发规则：

dream_0_sleep:
- 当前教室环境声在 2000ms 内降低到 20%
- 不播放 bgm_dream_air_walk

dream_1_fall:
- 播放 emo_dream_fall_low
- 播放 sfx_dream_whoosh_soft
- 可选播放 sfx_paper_fragment
- 本段结束时停止或淡出 emo_dream_fall_low
- 不播放 bgm_dream_air_walk

dream_2_suspend:
- 停止下坠音效
- 播放 amb_dream_air，fadeIn 1500ms
- 播放 bgm_dream_air_walk，fadeIn 2500ms

dream_3_future_choice / dream_3_future_far / dream_3_future_together / dream_3_future_create:
- 保持 amb_dream_air
- 保持 bgm_dream_air_walk
- 不新增其他音效

dream_4_end:
- bgm_dream_air_walk 音量降低到约 70%
- amb_dream_air 保持低音量

dream_5_wake:
- bgm_dream_air_walk fadeOut 3000ms
- amb_dream_air fadeOut 2000ms
- 播放 sfx_school_bell，fadeIn 1000ms
- 下课铃后恢复教室环境声
- 结束后继续进入 scene10

要求：
1. 所有音频必须通过 AudioManager 和 audio_manifest.json 的 ID 播放。
2. 不直接写文件路径。
3. 缺失音频只 warning，不崩溃。
4. 如果 AudioManager 暂不支持某个 fade 或 volume 操作，使用项目已有最小替代实现，并在报告中说明。
5. 不修改梦境文本。
6. 不修改梦境触发条件。
7. 不修改 shader。
8. 不影响其他场景音频。
9. 不修改场景10之后的流程。

完成后：
1. 将 dream_status.md 中 STEP8_DREAM_AUDIO 标记为 done。
2. 汇报：
   - 每个音频 ID 触发在哪个 dream scene
   - 每个音频是否循环
   - 每个淡入淡出时间
   - 缺失音频时如何 warning
   - 如何测试完整梦境音频

## STEP9_QA
请对梦境彩蛋做一次完整 QA 检查，不要新增功能。

请检查：
1. rest_flag_scene1=false, rest_flag_scene5=false，7B 选择睡觉：是否睡不着，不进入梦境。
2. rest_flag_scene1=true, rest_flag_scene5=false，7B 选择睡觉：是否睡不着，不进入梦境。
3. rest_flag_scene1=true, rest_flag_scene5=true，7B 选择睡觉：是否进入 dream_0_sleep。
4. 梦境从 dream_0_sleep 到 dream_5_wake 是否完整推进。
5. dream_5_wake 结束后是否进入 scene10。
6. side_route_mode 是否正确设置为 dream。
7. 主线 7A 是否不受影响。
8. 7B 写作业路线是否不受影响。
9. shader 关闭时梦境是否仍可正常运行。
10. 音频文件缺失时是否 warning 而不是崩溃。
11. 是否有硬编码绝对路径。
12. 是否有未使用、重复、死代码。

请输出：
- 检查结果
- 发现的问题
- 建议修复顺序

本轮只检查，不修复。

