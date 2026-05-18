# Dream Task Status

## 鎵ц瑙勫垯

Codex 姣忔鍙墽琛岀涓€涓姸鎬佷负 `todo` 鐨?STEP銆?鎵ц鍓嶆敼涓?`doing`銆?瀹屾垚鍚庢敼涓?`done`銆?鏃犳硶瀹屾垚鏀逛负 `blocked`銆?涓嶅緱璺宠繃锛屼笉寰楀悎骞讹紝涓嶅緱鑷姩缁х画鍚庣画 STEP銆?
## 鐘舵€佽〃

| STEP | 鐘舵€?| 璇存槑 |
|---|---|---|
| STEP0_AUDIT_CURRENT_DREAM | done | 宸插畬鎴愬綋鍓嶆ⅵ澧冨疄鐜板璁★紝缁撹瑙佸璁¤褰?|
| STEP1_CLEANUP_DREAM | done | 宸插畬鎴愭渶灏忔竻鐞嗭紝淇姊﹀鑳屾櫙璧勬簮鏄犲皠閿欒骞剁Щ闄や复鏃舵枃浠?|
| STEP2_PROJECT_STRUCTURE_ANALYSIS | done | 宸插畬鎴愬伐绋嬬粨鏋勫垎鏋愶紝缁撹瑙佺粨鏋勮褰?|
| STEP3_IMPLEMENTATION_PLAN | done | 宸插畬鎴愭渶灏忓疄鐜拌鍒掞紝鍙畾涔?dream scene/鍙橀噺/璺宠浆/鍚庣画鏀瑰姩杈圭晫 |
| STEP4_DREAM_SCENE_SEQUENCE | done | 宸叉柊澧炴渶灏?dream scene 搴忓垪锛屽苟鎺ュ埌 scene10_departure_bell |
| STEP4B_UPDATE_FUTURE_CHOICE | done | 宸叉敼涓轰笁閫夐」鏈潵纰庣墖妯″紡锛屽苟璁板綍宸茶鐘舵€佸悗姹囧叆 dream_4_end |
| STEP5_TRIGGER_CONDITION | done | 已核对 7B 睡觉触发条件已连通，无需再改运行时代码 |
| STEP6_BASIC_PRESENTATION | done | 已为 `dream_*` 接入基础黑场淡入淡出和 performance 分帧展示，不接 shader |
| STEP7A_SHADER_INFRA | done | 已复用并扩展现有 `ShaderToyWidget`，完成 dream shader overlay 基础设施 |
| STEP7B_DREAM_SHADER_BINDING | done | 已将两个 Shadertoy shader 绑定到对应 `dream_*` 场景，并保留失败回退 |
| STEP8_DREAM_AUDIO | done | 已接入梦境 BGM / 环境音 / 关键音效，并保留缺失资源 warning 回退 |
| STEP9_QA | done | 已完成梦境彩蛋 QA 检查，结论与残留问题见 STEP9 记录 |


## STEP0 瀹¤璁板綍

- 瀹¤鏃ユ湡锛?026-05-11
- 瀹¤鑼冨洿锛歚data/story_main.json`銆乣src/gamewindow.cpp`銆乣src/core/backgroundstyle.h`銆乣src/core/shadereffect.h`銆乣src/narrative/storyloader.cpp`銆乣src/ui/backgroundwidget.cpp`銆乣src/ui/shadertoywidget.cpp`
- 褰撳墠姊﹀瀹炵幇涓嶆槸鏋佺畝鐗堬紝鑰屾槸鈥滄暟鎹眰 scene + GameWindow 鐗瑰垽 + Dream 鑳屾櫙/Shader 鎵╁睍鈥濈殑娣峰悎瀹炵幇銆?- 姊﹀瑙﹀彂鍏ュ彛浠嶅湪 7B 鏀嚎涓紝鍒ゆ柇鏉′欢浣嶄簬 `story_main.json`锛歚rest_flag_scene1 == true` 涓?`rest_flag_scene5 == true` 鏃讹紝`scene7b_sleep_attempt -> scene7b_sleep_check_scene5 -> scene7b_dream_sleep`銆?- 褰撳墠姊﹀涓婚摼鍖呭惈锛歚scene7b_dream_sleep`銆乣scene7b_dream_fall`銆乣scene7b_dream_hover_intro`銆佸娈?`scene7b_dream_hover_*`銆乣scene7b_dream_fragment_*`銆乣scene7b_dream_close`銆乣scene7b_dream_wake`銆?- 褰撳墠瀹炵幇宸茬粡渚靛叆 7B 鏀嚎鍜屽満鏅?10/11/12 鐨勬枃鏈垎鏀紱鍙橀噺绯荤粺搴曞眰鏈涓撻棬閲嶆瀯锛屼絾鏂板骞朵娇鐢ㄤ簡 `side_route_mode`銆乣dream_hover_state`銆乣ending_tone` 绛夐敭銆?- 褰撳墠瀹炵幇瀛樺湪纭紪鐮佽祫婧愯矾寰勪笌鏂囦欢鍚嶆槧灏勶紝姊﹀璧勬簮鐢?`gamewindow.cpp` 鍐呴儴瀛楃涓叉槧灏勫埌 `鍥?姊?...`銆?- 宸插彂鐜版槑鏄鹃闄╋細鏃т唬鐮佸紩鐢ㄧ殑姊﹀鑳屾櫙鎵╁睍鍚嶄笌浠撳簱瀹為檯璧勬簮涓嶄竴鑷达紝瀛樺湪鑳屾櫙鍔犺浇澶辫触鍙兘銆?- 宸插彂鐜扮粨鏋勯闄╋細`gamewindow.cpp` 閲屾湁澶氬鍩轰簬 `scene7b_dream_*` 鐨勫垎鏁ｇ壒鍒わ紝鍚庣画娓呯悊鏃堕渶瑕佷紭鍏堝垽鏂摢浜涙槸蹇呴』淇濈暀鐨勫叆鍙ｆ帶鍒讹紝鍝簺灞炰簬鏃фⅵ澧冪殑鍐椾綑琛ㄧ幇灞傞€昏緫銆?
## STEP1 娓呯悊璁板綍

- 娓呯悊鏃ユ湡锛?026-05-11
- 淇敼鏂囦欢锛歚src/gamewindow.cpp`
- 鍒犻櫎鏂囦欢锛歚temp_dream_status_snapshot.txt`
- 宸蹭慨姝ｇ殑鏄庢樉閿欒锛?  - 灏?`DreamFaraway` 鑳屾櫙鏄犲皠浠?`姊?杩滄柟.jpg` 鏀逛负 `姊?杩滄柟.png`
  - 灏?`DreamCompanion` 鑳屾櫙鏄犲皠浠?`姊?鏈嬪弸.jpg` 鏀逛负 `姊?鏈嬪弸.png`
  - 灏?`DreamCreation` 鑳屾櫙鏄犲皠浠?`姊?鍒涗綔.jpg` 鏀逛负 `姊?鍒涗綔.png`
- 宸蹭繚鐣欑殑鍐呭锛?  - 褰撳墠 7B 姊﹀瑙﹀彂鍏ュ彛
  - 褰撳墠 `scene7b_dream_*` 鍦烘櫙閾?  - 鐜版湁 shader銆侀煶棰戙€丟ameWindow 姊﹀鐗瑰垽閫昏緫
- 淇濈暀鍘熷洜锛?  - 杩欎簺閫昏緫褰撳墠浠嶈鐜版湁姊﹀鍏ュ彛浣跨敤锛屾槸鍚﹀睘浜庡悗缁鏁翠綋鏇挎崲鐨勬棫瀹炵幇锛岄渶瑕佸湪鍚庣画 STEP 涓寜浠诲姟瑕佹眰閫愭澶勭悊锛涙湰杞笉鐩存帴鍒犳敼涓嶇‘瀹氫唬鐮併€?- 褰撳墠姊﹀鍏ュ彛鏄惁浠嶅瓨鍦細瀛樺湪銆?- 楠岃瘉缁撴灉锛?  - 宸茬‘璁?`鍥?姊?杩滄柟.png`銆乣鍥?姊?鏈嬪弸.png`銆乣鍥?姊?鍒涗綔.png` 涓変釜璧勬簮鏂囦欢鍧囧瓨鍦ㄣ€?  - 宸叉墽琛?`cmake --build build`锛屾瀯寤洪€氳繃銆?- 濡備綍娴嬭瘯 7B 鏀嚎娌℃湁鍧忥細
  - 杩涘叆 7B 鍒嗘敮锛屽垎鍒祴璇曗€滅户缁啓浣滀笟鈥濆拰鈥滅潯涓€瑙夆€濅袱鏉¤矾寰勩€?  - 鐫′竴瑙夎矾寰勮嚦灏戠‘璁よ兘缁х画杩涘叆鈥滅潯涓嶇潃鈥濇垨褰撳墠姊﹀鍏ュ彛锛屼笉搴斿嚭鐜版ⅵ澧冭儗鏅己澶卞鑷寸殑绌虹櫧鐢婚潰銆?
## STEP2 缁撴瀯璁板綍

- 鍒嗘瀽鏃ユ湡锛?026-05-11
- 鏈疆浠呭仛缁撴瀯鍒嗘瀽锛屾湭淇敼杩愯鏃朵唬鐮併€?- 宸ョ▼涓诲叆鍙ｏ細
  - `src/main.cpp` 鍙礋璐ｅ垱寤?`QApplication` 鍜?`GameWindow`銆?  - 姊﹀浠诲姟鐨勫疄闄呭叆鍙ｅ眰涓嶅湪 `main.cpp`锛岃€屽湪 `GameWindow` 鍔犺浇鏁呬簨鏁版嵁涔嬪悗鐨勮繍琛屾椂鍒嗗彂銆?- 鏋勫缓涓庤祫婧愭墦鍖咃細
  - `CMakeLists.txt` 鎶?`data/story_main.json`銆乣data/audio_manifest.json` 鍜?`鍥?`銆乣assets/audio/` 澶嶅埗鍒?`build/`銆?  - 杩欐剰鍛崇潃姊﹀濡傛灉缁х画璧版暟鎹┍鍔紝浼樺厛鏀?`data/story_main.json`锛屼笉鐢ㄦ敼鏋勫缓绯荤粺銆?- 鍙欎簨鏁版嵁灞傦細
  - `src/narrative/scene.h` 瀹氫箟 `NarrativeScene`銆乣NarrativeInteraction`銆佹潯浠惰烦杞€佽儗鏅€乻hader銆佽嚜鍔ㄦ帹杩涚瓑鍩虹缁撴瀯銆?  - `src/narrative/gamestate.h` 鍙彁渚涢€氱敤 `bool/string` 鍙橀噺瀛樺彇锛屾病鏈夋ⅵ澧冧笓鐢ㄧ姸鎬佹満銆?  - 缁撹锛氭ⅵ澧冪殑鏈€灏忔纭帴鍏ョ偣搴斾紭鍏堟槸 scene 鏁版嵁鍜岀幇鏈夊彉閲忥紝鑰屼笉鏄柊澧炵郴缁熺被銆?- 鏁呬簨鍔犺浇灞傦細
  - `src/narrative/storyloader.cpp` 璐熻矗鎶?JSON 瑙ｆ瀽鎴?`NarrativeScene`銆?  - 杩欓噷褰撳墠宸茬粡鏀寔姊﹀鐩稿叧鐨?`background` 鍜?`shader_effect` 瀛楃涓层€?  - 濡傛灉鍚庣画鐩爣鏄€滄櫘閫?dream scene 鏁版嵁鍜岃烦杞€濓紝涓昏渚濊禆杩欎釜鏃㈡湁鍔犺浇閾撅紝涓嶉渶瑕佹柊 loader銆?- 鍙欎簨杩愯灞傦細
  - `src/narrative/narrativeengine.cpp` 璐熻矗杩涘叆鍦烘櫙銆佸鐞嗕氦浜掋€佹墽琛屽彉閲忓啓鍏ャ€佸畬鎴?`next` / `next_variants` / `completion_next` 璺宠浆銆?  - 杩欓噷鐨勭粨鏋勫姊﹀鏄€氱敤鐨勶紝璇存槑 dream scene 搴忓垪鏈川涓婂彲浠ュ彧闈犳暟鎹眰閰嶇疆瀹屾垚銆?  - 褰撳墠杩愯灞傚凡缁忔敮鎸佽繍琛屾椂 shader 鍜?auto frame锛屼絾骞朵笉瑕佹眰姊﹀涓€瀹氫娇鐢ㄨ繖浜涘姛鑳姐€?- 绐楀彛涓庤〃鐜板眰锛?  - `src/gamewindow.cpp/.h` 鏄綋鍓嶆ⅵ澧冧镜鍏ユ渶娣辩殑鏂囦欢銆?  - 瀹冭礋璐ｏ細鑳屾櫙鍥炬煡鎵俱€乁I 鍒囨崲銆佽嚜鍔ㄦ帹杩涙椂闀裤€侀煶棰戠瓥鐣ャ€佽鑹茬珛缁樻樉绀恒€乻hader overlay 鍚仠銆?  - 褰撳墠澶ч噺 `scene7b_dream_*` 鐗瑰垽闆嗕腑鍦ㄨ繖閲岋紝鎵€浠ュ悗缁嚒鏄€滃彧鍋氭暟鎹簭鍒椻€濇垨鈥滃彧鍋氳Е鍙戞潯浠垛€濇椂锛屽簲灏介噺閬垮厤缁х画鍦ㄨ繖閲屾墿鏁ｇ壒鍒ゃ€?- UI 缁勪欢杈圭晫锛?  - `src/ui/dialoguepanel.*` 璐熻矗鏂囨湰銆侀€夐」銆佺户缁寜閽€?  - `src/ui/backgroundwidget.*` 璐熻矗鑳屾櫙搴曞浘鍜岃壊璋?娓愬彉銆?  - `src/ui/shadertoywidget.*` 鏄彲閫?shader 灞傦紝涓嶆槸 dream scene 鏈€灏忓疄鐜扮殑蹇呴渶璺緞銆?- 褰撳墠姊﹀鐩稿叧鏋氫妇鎵╁睍锛?  - `src/core/backgroundstyle.h` 宸叉柊澧?Dream 绯诲垪鑳屾櫙鏋氫妇銆?  - `src/core/shadereffect.h` 宸叉柊澧?Dream 绯诲垪 shader 鏋氫妇銆?  - 杩欒鏄庝粨搴撳綋鍓嶅凡缁忓厑璁告ⅵ澧冭蛋鈥滀笓鐢ㄨ儗鏅?涓撶敤 shader鈥濊矾绾匡紝浣嗗悗缁楠ゅ鏋滆姹傛渶灏忓疄鐜帮紝鍙互鍙鐢ㄨ儗鏅€佷笉缁х画鎵╁睍鏋氫妇銆?- 姊﹀浠诲姟鐨勬帹鑽愭渶灏忔敼鍔ㄩ『搴忥細
  1. 鍏堝湪 `data/story_main.json` 涓暣鐞嗘垨鏇挎崲 dream scene 搴忓垪銆?  2. 鍐嶅彧鏀?7B 鍏ュ彛鍒ゆ柇锛屾妸瑙﹀彂鏉′欢鎺ュ埌鏂扮殑 dream 搴忓垪銆?  3. 鐒跺悗鍐嶈浠诲姟瑕佹眰鍐冲畾鏄惁澶嶇敤 `GameWindow` 閲屽凡鏈夌殑 performance / fade / shader 鑳藉姏銆?- 褰撳墠鏈€搴旈伩鍏嶇殑鏀瑰姩鏂瑰悜锛?  - 涓嶈鍏堟敼 `AudioManager`
  - 涓嶈鍏堟敼 `SceneManager` 椋庢牸缁撴瀯
  - 涓嶈鍏堝湪 `NarrativeEngine` 閲屽姞姊﹀涓撶敤閫昏緫
  - 涓嶈缁х画澧炲姞鏂扮殑鍏ㄥ眬鐘舵€佸鍣?- 瀵瑰悗缁?STEP 鐨勭洿鎺ョ粨璁猴細
  - `STEP3_IMPLEMENTATION_PLAN` 鐨勯噸鐐瑰簲钀藉湪 `data/story_main.json`銆?B 鍏ュ彛鍙橀噺銆佹槸鍚﹀鐢ㄧ幇鏈?`BackgroundStyle` / `ShaderEffect`銆?  - 褰撳墠瀹炵幇閲岋紝`gamewindow.cpp` 鏄珮椋庨櫓鏂囦欢锛涘悗缁嫢闈炰换鍔℃槑纭姹傦紝灏介噺灏戞敼銆?- 涓嬩竴姝ュ簲鎵ц锛歚STEP3_IMPLEMENTATION_PLAN`銆?
## STEP3 瀹炵幇璁″垝璁板綍

- 璁″垝鏃ユ湡锛?026-05-11
- 鏈疆鍙啓璁″垝锛屼笉鏀逛唬鐮侊紝涓嶆敼鍓ф儏鏂囨湰锛屼笉鏀?`AudioManager`銆乣NarrativeEngine` 涓绘祦绋嬨€乣GameWindow` 姊﹀鐗瑰垽銆?- 鐩爣锛氭妸褰撳墠澶嶆潅鐨?`scene7b_dream_*` 鏃ч摼璺紝鏀舵暃鎴愪竴鏉♀€滅函 scene 鏁版嵁椹卞姩鈥濈殑鏈€灏忔ⅵ澧冨簭鍒楋紝涓轰笅涓€姝ュ彧鏀?`story_main.json` 鍋氬噯澶囥€?
- 1. 闇€瑕佹柊澧炴垨鏇挎崲鐨?scene ID锛?  - `dream_0_sleep`
  - `dream_1_fall`
  - `dream_2_suspend`
  - `dream_3_future_room`
  - `dream_3_future_road`
  - `dream_3_future_create`
  - `dream_4_end`
  - `dream_5_wake`
- 璇存槑锛?  - 涓嬩竴姝ヤ紭鍏堢洿鎺ュ湪 `data/story_main.json` 涓娇鐢ㄨ繖缁勬柊 ID銆?  - 鏃х殑 `scene7b_dream_hover_*`銆乣scene7b_dream_fragment_*` 涓嶅湪 STEP4 缁х画澶嶇敤锛涚瓑鏂伴摼璺窇閫氬悗锛屽啀鍐冲畾鏄惁鍒犻櫎鏃ч摼璺€?
- 2. 闇€瑕佹柊澧?/ 澶嶇敤鐨勫彉閲忥細
  - 澶嶇敤 `rest_flag_scene1`
  - 澶嶇敤 `rest_flag_scene5`
  - 澶嶇敤 `side_route_mode`
  - 涓嶅啀渚濊禆 `dream_hover_state`
- 璇存槑锛?  - `rest_flag_scene1`銆乣rest_flag_scene5` 鍙敤浜?7B 鐫¤鍏ュ彛鏉′欢銆?  - `side_route_mode` 鍦ㄦⅵ澧冩垚鍔熻繘鍏ユ椂鍐欐垚 `"dream"`锛岀户缁緵 `scene10`銆乣scene11`銆乣scene12` 鍚庣画鏂囨湰鍒嗘敮澶嶇敤銆?  - 鏃у€?`"姊﹀"` 鏄惁浠嶈鐜版湁鏂囨湰鍒ゆ柇浣跨敤锛岄渶瑕佸湪 STEP4 钀藉湴鍓嶄繚鎸佷竴鑷存€э紱濡傛灉鐜版湁 `scene10+` 浠嶅垽鏂?`"姊﹀"`锛屽垯 STEP4 鍏堢户缁部鐢?`"姊﹀"`锛岀瓑鏁存潯閾捐矾绋冲畾鍚庡啀缁熶竴銆?
- 3. 姣忎釜 scene 鐨勮繘鍏ユ潯浠讹細
  - `dream_0_sleep`锛氫粎鍦?7B 閫夋嫨鈥滅潯涓€瑙夆€濅笖 `rest_flag_scene1 == true` 涓?`rest_flag_scene5 == true` 鏃惰繘鍏ャ€?  - `dream_1_fall`锛氫粠 `dream_0_sleep` 鍥哄畾杩涘叆銆?  - `dream_2_suspend`锛氫粠 `dream_1_fall` 鍥哄畾杩涘叆銆?  - `dream_3_future_room`锛氫粠 `dream_2_suspend` 鍥哄畾杩涘叆銆?  - `dream_3_future_road`锛氫粠 `dream_3_future_room` 鍥哄畾杩涘叆銆?  - `dream_3_future_create`锛氫粠 `dream_3_future_road` 鍥哄畾杩涘叆銆?  - `dream_4_end`锛氫粠 `dream_3_future_create` 鍥哄畾杩涘叆銆?  - `dream_5_wake`锛氫粠 `dream_4_end` 鍥哄畾杩涘叆銆?
- 4. 姣忎釜 scene 鐨勪笅涓€涓烦杞細
  - `dream_0_sleep -> dream_1_fall`
  - `dream_1_fall -> dream_2_suspend`
  - `dream_2_suspend -> dream_3_future_room`
  - `dream_3_future_room -> dream_3_future_road`
  - `dream_3_future_road -> dream_3_future_create`
  - `dream_3_future_create -> dream_4_end`
  - `dream_4_end -> dream_5_wake`
  - `dream_5_wake -> scene10_departure_bell`
- 璇存槑锛?  - 涓嶄娇鐢ㄦ偓鍋滈€夋嫨銆佺鐗囧洖鐜€佸鍒嗘敮鍥炶烦銆?  - 鏁存潯姊﹀閾惧彧淇濈暀鍗曠嚎鎺ㄨ繘锛岄伩鍏嶇户缁緷璧?`dream_hover_state` 鍜?`GameWindow` 涓鏃?`scene7b_dream_*` 鍓嶇紑鐨勫垎鏁ｇ壒鍒ゃ€?
- 5. 闇€瑕佺殑婕斿嚭鍛戒护杈圭晫锛?  - STEP4 鍙娇鐢ㄦ櫘閫?`mode`銆乣background`銆乣text`銆乣next`銆佸繀瑕佺殑 `set_string`銆?  - 鏆備笉鎺ュ叆 `shader_effect`
  - 鏆備笉鎺ュ叆鏂伴煶鏁?  - 鏆備笉鏂板澶嶆潅 `performance` 瀹氭椂瑙勫垯
  - 濡傞渶榛戝睆鍗犱綅锛屼紭鍏堝鐢ㄧ幇鏈夊彲鐢ㄨ儗鏅垨褰撳墠椤圭洰宸叉湁鐨勬ⅵ澧冨崰浣嶈儗鏅紝涓嶆柊澧炴紨鍑虹郴缁?
- 6. 鍝簺鍦版柟鍏堢敤鍗犱綅瀹炵幇锛?  - `dream_0_sleep`锛氬彲鍏堝鐢?`desk_night`
  - `dream_1_fall`锛氬彲鍏堝鐢ㄩ粦灞忔垨鏈€鎺ヨ繎榛戝満鐨勭幇鏈夎儗鏅?  - `dream_2_suspend` 鍒?`dream_4_end`锛氬彲鍏堢粺涓€澶嶇敤涓€涓凡鏈夋ⅵ澧冨崰浣嶈儗鏅紝涓嶅尯鍒嗕笁寮犳湭鏉ョ鐗囦笓灞炶儗鏅?  - `dream_5_wake`锛氬厛澶嶇敤 `desk_night`
- 璇存槑锛?  - 鏈潵鎴块棿 / 璺?/ 鍒涗綔 涓夋鍦?STEP4 鍏堥潬鏂囨湰鍖哄垎锛屼笉寮轰緷璧栧崟鐙祫婧愩€?  - 杩欐牱鑳芥妸椋庨櫓闆嗕腑鍦ㄢ€渟cene 鏄惁鑳借窇閫氣€濓紝鑰屼笉鏄崱鍦ㄨ儗鏅€乻hader銆侀煶棰戣祫婧愪竴鑷存€т笂銆?
- 7. 鏈€灏忎唬鐮佹敼鍔ㄦ柟妗堬細
  - STEP4 鍙敼 `data/story_main.json`
  - 鍦?7B 闄勮繎鏂板鎴栨浛鎹㈡垚鏂扮殑 `dream_*` scene 搴忓垪
  - 鍏堜繚鐣欏師鏈夆€滃啓浣滀笟鈥濅笌鈥滅潯涓嶇潃鈥濊矾绾夸笉鍔?  - `dream_5_wake` 鏈熬鐩存帴璺?`scene10_departure_bell`
  - `side_route_mode` 鐨勫啓鍏ユ斁鍦?`dream_0_sleep` 鎴?`dream_5_wake`
- 寤鸿锛?  - 涓洪伩鍏嶅奖鍝?`scene10` 鏃㈡湁鍒嗘敮鏂囨湰锛孲TEP4 鍏堟妸鍐欏叆鍊间繚鎸佹垚褰撳墠椤圭洰姝ｅ湪浣跨敤鐨勫€硷紱鑻ョ幇鏈変笅娓稿垽鏂繕鏄?`"姊﹀"`锛屽氨鍏堢户缁啓 `"姊﹀"`锛屼笉瑕佸湪 STEP4 鍚屾椂鏀逛笅娓告枃妗堟潯浠躲€?  - `GameWindow`銆乣backgroundstyle`銆乣shadereffect`銆乣storyloader` 鏆備笉淇敼锛涘彧鏈夊綋 STEP6 / STEP7 鏄庣‘闇€瑕佹紨鍑哄寮烘椂鍐嶈瘎浼般€?
- 椋庨櫓鎻愮ず锛?  - 褰撳墠 `gamewindow.cpp` 寰堝彲鑳戒粛淇濈暀瀵规棫 `scene7b_dream_*` 鐨勮嚜鍔ㄦ帹杩涖€侀煶棰戙€乻hader 鐗瑰垽锛涘鏋?STEP4 鏀圭敤 `dream_*` 鏂?ID锛岃繖浜涚壒鍒ゅぇ姒傜巼涓嶄細鑷姩鐢熸晥銆?  - 杩欏湪 STEP4 涓嶆槸闃诲闂锛屽洜涓?STEP4 鐨勭洰鏍囨湰鏉ュ氨鏄€滃厛璺戦€氭櫘閫?scene 鏁版嵁鍜岃烦杞€濓紱婕斿嚭缂哄け鐣欏埌鍚庣画 STEP 澶勭悊鏇村畨鍏ㄣ€?
- 鏈疆缁撹锛?  - STEP4 搴旇鍙姩 `data/story_main.json`锛屾妸姊﹀鍏堟敹缂╂垚 8 涓崟绾?scene銆?  - STEP5 鍐嶅彧鏀?7B 鐫¤鏉′欢鎺ュ叆銆?  - 涓嶅缓璁湪 STEP4 鎻愬墠娓呯悊鏃?`gamewindow.cpp` 姊﹀鐗瑰垽锛屽惁鍒欎細鎶娾€滄暟鎹敼閫犫€濆拰鈥滆繍琛屾椂娓呯悊鈥濇贩鎴愪竴涓楠ゃ€?
- 涓嬩竴姝ュ簲鎵ц锛歚STEP4_DREAM_SCENE_SEQUENCE`

## STEP4 姊﹀鍦烘櫙搴忓垪璁板綍

- 鎵ц鏃ユ湡锛?026-05-11
- 鏈疆鍙慨鏀?`data/story_main.json` 涓殑姊﹀ scene 鏁版嵁涓庤烦杞紝鏈慨鏀?`GameWindow`銆乣AudioManager`銆乣NarrativeEngine`銆?A銆佸満鏅?10/11/12 鐨勬鏂囧唴瀹广€?- 鏈疆鍋氭硶锛?  - 淇濈暀鐜版湁 7B 鐫¤鍒ゆ柇缁撴瀯銆?  - 灏?`scene7b_sleep_check_scene5` 鎴愬姛杩涘叆姊﹀鏃剁殑鐩爣浠庢棫 `scene7b_dream_sleep` 鏀逛负鏂?`dream_0_sleep`銆?  - 鏂板 8 涓崟绾?dream scene锛?    - `dream_0_sleep`
    - `dream_1_fall`
    - `dream_2_suspend`
    - `dream_3_future_room`
    - `dream_3_future_road`
    - `dream_3_future_create`
    - `dream_4_end`
    - `dream_5_wake`

- 鏂?dream scene 鐨勮烦杞摼锛?  - `dream_0_sleep -> dream_1_fall`
  - `dream_1_fall -> dream_2_suspend`
  - `dream_2_suspend -> dream_3_future_room`
  - `dream_3_future_room -> dream_3_future_road`
  - `dream_3_future_road -> dream_3_future_create`
  - `dream_3_future_create -> dream_4_end`
  - `dream_4_end -> dream_5_wake`
  - `dream_5_wake -> scene10_departure_bell`

- 鏈疆鐨勮儗鏅笌琛ㄧ幇杈圭晫锛?  - `dream_0_sleep`銆乣dream_5_wake` 澶嶇敤 `desk_night`
  - `dream_1_fall` 鍒?`dream_4_end` 缁熶竴澶嶇敤 `dream_drift`
  - 鍏ㄩ儴浣跨敤鏅€?`narration` 鎺ㄨ繘
  - 鏈柊澧?`shader_effect`
  - 鏈柊澧為煶棰?  - 鏈柊澧炲鏉傝嚜鍔ㄦ帹杩涜鍒?
- 鍙橀噺澶勭悊锛?  - 鍦?`dream_0_sleep` 缁х画鍐欏叆 `side_route_mode`
  - 涓轰簡涓嶆敼鍔ㄥ満鏅?10/11/12 鐜版湁鍒嗘敮鍒ゆ柇锛屾湰杞部鐢ㄥ綋鍓嶉」鐩笅娓稿凡浣跨敤鐨勫€?`"姊﹀"`锛屾湭鍦ㄦ湰杞垏鎹㈡垚 `"dream"`

- 淇濈暀浣嗘湭娓呯悊鐨勬棫鍐呭锛?  - 鏃?`scene7b_dream_*` 澶嶆潅姊﹀閾句粛鐣欏湪 `story_main.json` 涓紝浣嗘湰杞柊澧炲叆鍙ｅ凡浼樺厛璺冲埌鏂扮殑 `dream_*` 鍗曠嚎搴忓垪
  - 杩欐牱鍋氭槸涓轰簡鎶娾€滄柊閾捐矾钀藉湴鈥濆拰鈥滄棫閾捐矾娓呯悊鈥濆垎寮€锛岄伩鍏嶈法 STEP 娣峰仛

- 鏈疆楠岃瘉锛?  - 宸茬敤 `rg` 纭锛?    - `scene7b_sleep_check_scene5` 鐜板湪璺冲埌 `dream_0_sleep`
    - 8 涓柊 scene ID 宸插瓨鍦?    - `dream_5_wake` 宸茶烦鍒?`scene10_departure_bell`
  - 灏濊瘯浣跨敤 PowerShell `ConvertFrom-Json` 鏍￠獙鏁翠唤 `story_main.json` 鏃跺け璐ワ紝浣嗗け璐ヤ綅缃潵鑷枃浠朵腑鏇存棭鐨勬棦鏈夊唴瀹癸紱鍥犳鏈疆鏈妸瀹冧綔涓烘柊澧炴ⅵ澧冩鐨勭嫭绔嬮樆濉為棶棰樺鐞?  - 鏈疆鏈繍琛岀紪璇戯紝鍥犱负娌℃湁鏀?C++ 浠ｇ爜

- 濡備綍娴嬭瘯鐩存帴杩涘叆姊﹀锛?  - 浠庢甯告祦绋嬭繘鍏?7B锛屾弧瓒充紤鎭潯浠跺悗閫夋嫨鈥滅潯涓€瑙夆€?  - 濡傛灉鍙兂鏈湴蹇€熼獙璇?dream 搴忓垪鏈韩锛屽彲涓存椂璁╁綋鍓嶆祴璇曞叆鍙?`next` 鎸囧悜 `dream_0_sleep`锛涙湰杞湭杩欐牱鏀逛粨搴撳唴瀹?
- 濡備綍娴嬭瘯姊﹀缁撴潫鍚庤繘鍏?scene10锛?  - 浠?`dream_0_sleep` 杩炵画鎺ㄨ繘鍒?`dream_5_wake`
  - 纭涓嬩竴鍦烘櫙涓?`scene10_departure_bell`
  - 鍐嶇‘璁ゅ満鏅?10 鍚庣画浠嶆部鐢?`side_route_mode = "姊﹀"` 鐨勭幇鏈夋枃鏈垎鏀?
- 涓嬩竴姝ュ簲鎵ц锛歚STEP5_TRIGGER_CONDITION`

## STEP4B 鏈潵纰庣墖涓夐€夐」璁板綍

- 鎵ц鏃ユ湡锛?026-05-11
- 鏈疆鍙慨鏀?`data/story_main.json` 鍜?`docs/tasks/dream_status.md`銆?- 鏈疆鏈慨鏀?`GameWindow`銆乣AudioManager`銆乣NarrativeEngine` 涓绘祦绋嬨€?A銆佸満鏅?8銆佸満鏅?10/11/12 姝ｆ枃銆乻hader 鎴栭煶棰戙€?
- 淇敼缁撴灉锛?  - `dream_2_suspend` 涓嶅啀杩涘叆椤哄簭鎾斁鐗囨锛岃€屾槸杩涘叆 `dream_3_future_choice`
  - 鏂板 `dream_3_future_together`
  - 灏嗗師 `dream_3_future_road` 鏀逛负 `dream_3_future_far`
  - 鍘?`dream_3_future_room` 宸蹭笉鍐嶈繘鍏ワ紝褰撳墠 V1 鏈潵纰庣墖涓嶅啀浣跨敤璇ョ墖娈?  - `dream_3_future_create` 缁х画澶嶇敤锛屼絾涓嶅啀鐩存帴璺?`dream_4_end`

- 鏂板 / 浣跨敤鐨勫彉閲忥細
  - `dream_seen_far`
  - `dream_seen_together`
  - `dream_seen_create`
  - `dream_choice_state`

- 鏂板 / 淇敼鐨?scene ID锛?  - `dream_3_future_choice`
  - `dream_3_future_choice_after_far`
  - `dream_3_future_choice_after_together`
  - `dream_3_future_choice_after_create`
  - `dream_3_future_choice_after_far_together`
  - `dream_3_future_choice_after_far_create`
  - `dream_3_future_choice_after_together_create`
  - `dream_3_future_together`
  - `dream_3_future_far`
  - `dream_3_future_create`
  - `dream_2_suspend`

- 涓変釜閫夐」濡備綍鍒ゆ柇鏄惁宸茶锛?  - 鍦?`dream_2_suspend` 杩涘叆鏈潵閫夋嫨鍓嶏紝鍏堥噸缃細
  - `dream_seen_far = false`
  - `dream_seen_together = false`
  - `dream_seen_create = false`
  - `dream_choice_state = ""`
  - 鐐瑰嚮鈥滆繙鏂光€濇椂鍐欏叆 `dream_seen_far = true`
  - 鐐瑰嚮鈥滃苟鑲┾€濇椂鍐欏叆 `dream_seen_together = true`
  - 鐐瑰嚮鈥滃垱浣溾€濇椂鍐欏叆 `dream_seen_create = true`
  - 鍚屾椂鍐欏叆 `dream_choice_state`锛岀敤浜庡洖鍒板彧鍓╂湭璇婚」鐨?choice scene
  - 鍥犳鍥炴祦鍚庝笉浼氬啀鏄剧ず宸茬粡鐐硅繃鐨勯€夐」

- 涓変釜閫夐」鍏ㄩ儴瀹屾垚鍚庡浣曡繘鍏?`dream_4_end`锛?  - 鏈€鍚庝竴涓€夐」琚偣鍑绘椂锛屽皢 `dream_choice_state` 鍐欐垚 `"all"`
  - `dream_3_future_far`銆乣dream_3_future_together`銆乣dream_3_future_create` 閮芥牴鎹?`dream_choice_state` 鍋?`next_variants`
  - 褰?`dream_choice_state == "all"` 鏃讹紝鐗囨璇诲畬鐩存帴杩涘叆 `dream_4_end`

- 鏈疆楠岃瘉锛?  - 宸茬敤 `rg` 纭锛?  - `dream_2_suspend` 鐜板湪璺冲埌 `dream_3_future_choice`
  - 涓変釜 `dream_seen_*` 鍙橀噺鍜?`dream_choice_state` 閮藉凡鎺ュ叆
  - `dream_3_future_far`銆乣dream_3_future_together`銆乣dream_3_future_create` 閮藉瓨鍦ㄥ苟甯︽湁鍥炴祦 / 鏀舵潫璺宠浆
  - 宸茬‘璁?`dream_3_future_room`銆乣dream_3_future_road` 涓嶅啀浣滀负褰撳墠娲昏穬鏈潵纰庣墖鍏ュ彛
  - 宸插皾璇曠敤 PowerShell `ConvertFrom-Json` 鏍￠獙鏁翠唤 `story_main.json`
  - 璇ユ牎楠屼粛澶辫触锛屼絾澶辫触鐐逛綅浜庢枃浠跺墠閮ㄦ棦鏈夊唴瀹癸紝涓嶆槸鏈疆鏂板鐨勬湭鏉ョ鐗囨锛涘洜姝ゆ湰杞皢鍏惰褰曚负浠撳簱鐜版湁 JSON 鍏煎鎬ч棶棰橈紝涓嶄綔涓哄綋鍓?STEP 鐨勬柊澧為樆濉?
- 濡備綍娴嬭瘯鏈潵纰庣墖閫夋嫨閫昏緫锛?  - 杩涘叆 7B 姊﹀璺嚎锛屾帹杩涘埌 `dream_2_suspend`
  - 纭涓嬩竴姝ヨ繘鍏?`dream_3_future_choice`
  - 绗竴娆″簲鐪嬪埌 3 涓€夐」锛歚杩滄柟` / `骞惰偐` / `鍒涗綔`
  - 浠绘剰鐐瑰嚮涓€涓悗锛岃瀹岀墖娈靛簲鍥炲埌鍙墿 2 涓€夐」鐨勫搴?choice scene
  - 鍐嶇偣绗簩涓紝璇诲畬鍚庡簲鍥炲埌鍙墿 1 涓€夐」鐨勫搴?choice scene
  - 鐐瑰畬鏈€鍚庝竴涓悗锛屼笉鍐嶅洖鍒?choice锛岃€屾槸鐩存帴杩涘叆 `dream_4_end`

- 涓嬩竴姝ュ簲鎵ц锛歚STEP5_TRIGGER_CONDITION`

## STEP5 触发条件记录

- 执行日期：2026-05-11
- 本轮结论：当前 7B 睡觉分支已经接入梦境触发条件，本轮无需修改运行时代码。
- 已核对的触发链：
  - `scene7b_sleep_attempt` 先判断 `rest_flag_scene1 == true`
  - 命中后进入 `scene7b_sleep_check_scene5`
  - `scene7b_sleep_check_scene5` 再判断 `rest_flag_scene5 == true`
  - 两个条件都满足时进入 `dream_0_sleep`
  - 任一条件不满足时进入 `scene7b_sleep_fail_bell -> scene7b_sleep_fail`
- 已核对的变量写入：
  - 梦境成功入口会写入 `side_route_mode = "梦境"`
  - 睡不着分支会写入 `side_route_mode = "睡不着"`
- 本轮未修改内容：
  - 未改 `data/story_main.json` 的现有触发逻辑
  - 未改 `GameWindow`、`AudioManager`、`NarrativeEngine`、主线 7A、场景 8、场景 10/11/12
- 验证方式：使用 `rg` 回查 `rest_flag_scene1`、`rest_flag_scene5`、`scene7b_sleep_attempt`、`scene7b_sleep_check_scene5`、`dream_0_sleep`、`side_route_mode` 的连接关系，确认已符合当前任务要求。
- 下一步应执行：`STEP6_BASIC_PRESENTATION`

## STEP6 基础演出记录

- 执行日期：2026-05-11
- 本轮修改文件：
  - `src/gamewindow.cpp`
  - `src/gamewindow.h`
  - `data/story_main.json`
  - `docs/tasks/dream_status.md`
- 复用的现有演出接口：
  - 复用 `performance` 场景的自动分帧推进
  - 复用 `BackgroundStyle` 里的 `desk_night`、`dream_drift`、`ending_black`
  - 复用 `GameWindow` 现有的 `QGraphicsOpacityEffect` / `QPropertyAnimation` 能力做最小黑场覆盖层
- 本轮新增的最小演出支持：
  - 为新 `dream_*` 场景接入黑场淡入淡出 overlay
  - `dream_0_sleep` 进入后从教室逐渐压暗到黑场
  - `dream_1_fall` 改为 `ending_black` 背景，并用 `performance` 分句自动推进
  - `dream_2_suspend` 从黑场淡回 `dream_drift`
  - `dream_3_future_far` / `dream_3_future_together` / `dream_3_future_create` 进入时做短淡入
  - `dream_4_end` 在结尾逐渐淡出到黑场
  - `dream_5_wake` 从黑场淡回教室背景
- 本轮未修改内容：
  - 未接入 shader
  - 未接入音效
  - 未改 7B 触发条件
  - 未改主线 7A、场景 8、场景 10/11/12 的剧情文本
  - 未改 `AudioManager`、`NarrativeEngine` 主流程、全局 SceneManager 架构
- 是否影响其他场景：
  - 新增黑场 overlay 只在新 `dream_*` 场景切换时触发，离开梦境会清空，不影响其他场景转场
  - 旧 `scene7b_dream_*` 特判仍保留，本轮没有把它们并入新演出逻辑
- 本轮验证：
  - 执行 `cmake --build build`，构建通过
  - 回读 `dream_*` 段，确认 `dream_0_sleep`、`dream_1_fall`、`dream_2_suspend`、`dream_3_future_*`、`dream_4_end`、`dream_5_wake` 已切到 `performance` 或对应演出背景
  - 回读 `GameWindow`，确认新黑场 overlay 只绑定到新 `dream_*` scene
- 如何单独测试梦境演出：
  - 从正常流程进入 7B，满足休息条件后选择“睡一觉”
  - 观察 `dream_0_sleep` 是否从教室渐暗到黑场
  - 观察 `dream_1_fall` 是否为黑场背景、短句逐句出现
  - 观察 `dream_2_suspend` 是否从黑场淡回 `dream_drift`
  - 点开三个未来碎片，确认每次进入碎片都有短淡入
  - 观察 `dream_4_end -> dream_5_wake` 是否先淡黑再淡回教室
- 下一步应执行：`STEP7A_SHADER_INFRA`

## STEP7A Shader 基础设施记录

- 执行日期：2026-05-11
- 本轮修改文件：
  - `src/ui/shadertoywidget.h`
  - `src/ui/shadertoywidget.cpp`
  - `src/gamewindow.h`
  - `src/gamewindow.cpp`
  - `CMakeLists.txt`
  - `docs/tasks/dream_status.md`
- 本轮新增文件：
  - 无
- 本轮实现范围：
  - 复用并扩展现有 `ShaderToyWidget`，作为 dream shader overlay 的基础设施
  - 支持按文件路径加载外部 fragment shader，而不只依赖原有内置 `ShaderEffect` 枚举
  - 为外部 shader 提供 `iTime`、`iResolution`、`iMouse`、`iChannel0` 等 Shadertoy 风格 uniform
  - 若 shader 文件只定义 `mainImage(...)` 而没有 `main(...)`，自动补一层包装入口
- 本轮新增接口：
  - `ShaderToyWidget::setDreamShaderEnabled(bool enabled)`
  - `ShaderToyWidget::setExternalFragmentShaderFile(const QString &path)`
  - `GameWindow::setDreamShaderOverlayFile(const QString &shaderFilePath)`
  - `GameWindow::setDreamShaderOverlayEnabled(bool enabled)`
  - `GameWindow::clearDreamShaderOverlay()`
- `iChannel0` 当前处理方式：
  - 优先复用当前背景图生成的 source texture
  - 若当前没有可用背景纹理，则退回现有 1x1 深色 fallback texture
- 编译失败回退：
  - `ShaderToyWidget` 新增 `shaderCompileFailed(...)` 信号
  - `GameWindow` 监听该信号后会调用 `clearDreamShaderOverlay()`，退回普通背景显示
  - 同时输出 warning，避免静默黑屏
- 交互安全性：
  - shader overlay 增加 `Qt::WA_TransparentForMouseEvents`
  - 不会阻挡文本点击、选项点击或继续按钮
- 构建与资源复制：
  - `CMakeLists.txt` 已补充复制 `assets/shaders` 到 `build/assets`
  - 这样后续绑定梦境 shader 文件时不需要再改构建步骤
- 本轮未修改内容：
  - 未把具体 dream scene 绑定到某个 shader 文件
  - 未改主线 7A、场景 8、场景 10/11/12 文本
  - 未改 `AudioManager`
  - 未改 `NarrativeEngine` 主流程
  - 未改 7B 触发条件
- 本轮验证：
  - 执行 `cmake --build build`，构建通过
  - 回查确认外部 shader 文件接口、编译失败回退、overlay 清理逻辑已接入
  - 回查确认 `assets/shaders` 已复制到构建目录
- 下一步应执行：`STEP7B_DREAM_SHADER_BINDING`

## STEP7B Dream Shader 绑定记录

- 执行日期：2026-05-11
- 本轮修改文件：
  - `src/gamewindow.h`
  - `src/gamewindow.cpp`
  - `docs/tasks/dream_status.md`
- 本轮新增文件：
  - 无
- 本轮 shader 绑定结果：
  - `dream_1_fall` -> `dream_fall_xskgrw.frag`
  - `dream_2_suspend` -> `dream_future_mtcgdf.frag`
  - `dream_3_future_choice*` -> `dream_future_mtcgdf.frag`
  - `dream_3_future_far` -> `dream_future_mtcgdf.frag`
  - `dream_3_future_together` -> `dream_future_mtcgdf.frag`
  - `dream_3_future_create` -> `dream_future_mtcgdf.frag`
  - `dream_4_end` / `dream_5_wake` / 离开梦境 -> 关闭 dream shader overlay
- shader 开关位置：
  - 在 `GameWindow::applyDreamShaderBinding(...)` 中按当前 `sceneId` 决定
  - 通过 `setDreamShaderOverlayFile(...)` + `setDreamShaderOverlayEnabled(true)` 开启
  - 通过 `clearDreamShaderOverlay()` 关闭
- shader 文件路径处理：
  - 由于当前仓库里的 `assets/shaders` 目录存在嵌套层级不规整的问题
  - 本轮未重构资源目录
  - 改为在 `assets/shaders` 下递归查找 `dream_fall_xskgrw.frag` 和 `dream_future_mtcgdf.frag`
  - 这样构建目录中的复制结果仍可被定位并加载
- 选项如何叠加在 shader 上方：
  - `ShaderToyWidget` 仍作为背景上的 overlay
  - 开启 shader 后会重新 `raise()` 文本头、中心文本和 `DialoguePanel`
  - 因此 `dream_3_future_choice*` 的选项按钮会显示在 shader 上方，不会被遮住
- 三个未来碎片如何记录已读：
  - 本轮未改动既有变量逻辑
  - 继续复用 `dream_seen_far`、`dream_seen_together`、`dream_seen_create` 与 `dream_choice_state`
  - 已读判断和收束仍由 `data/story_main.json` 里的 choice scene / `next_variants` 负责
- `dream_1_fall` 的约 5 秒显示：
  - 本轮未改梦境文本和自动推进机制
  - 当前 `dream_1_fall` 仍使用既有 `performance` 分句推进
  - shader 会在该 scene 整段期间保持开启，并在切到 `dream_2_suspend` 时切换到 future shader
- shader 失败如何回退：
  - 若 shader 文件找不到，会输出 warning 并直接 `clearDreamShaderOverlay()`
  - 若 shader 编译失败，会走 STEP7A 已接好的 `shaderCompileFailed(...) -> clearDreamShaderOverlay()` 回退链
  - 因此梦境仍可退回普通背景继续推进
- 本轮未修改内容：
  - 未改梦境文本
  - 未改 7B 触发条件
  - 未改 `AudioManager`
  - 未改主线 7A、场景 8、场景 10/11/12
  - 未改未来碎片选择变量系统
- 本轮验证：
  - 执行 `cmake --build build`，构建通过
  - 回查确认 `dream_1_fall` 绑定 fall shader
  - 回查确认 `dream_2_suspend` 和所有 `dream_3_future_*` / `dream_3_future_choice*` 绑定 future shader
  - 回查确认 `dream_4_end`、`dream_5_wake` 与非梦境 scene 会清理 overlay
- 如何测试 `dream_1_fall` 的 shader：
  - 从正常流程进入 7B，满足休息条件后选择“睡一觉”
  - 推进到 `dream_1_fall`
  - 确认黑底坠落 shader 出现，并在切到 `dream_2_suspend` 时更换为另一套 future shader
- 如何测试三个未来选项全部完成后进入 `dream_4_end`：
  - 进入 `dream_2_suspend` 后确认 future shader 已开启
  - 在 `dream_3_future_choice` 依次点完三个选项
  - 确认三个 choice scene 始终显示在 shader 上方
  - 最后一个碎片读完后进入 `dream_4_end`
- 进入 `dream_4_end` 时确认 dream shader 已关闭，只保留原有淡出表现
- 下一步应执行：`STEP8_DREAM_AUDIO`

### STEP7 补充记录：更换简化 Shadertoy 后的返工

- 执行日期：2026-05-11
- 背景：
  - 用户已替换 `dream_fall_xskgrw.frag` 与 `dream_future_mtcgdf.frag`
  - 新文件均只保留一个 `mainImage(...)`，已不再是之前那种多段/重复入口结构
- 本轮修改文件：
  - `src/ui/shadertoywidget.h`
  - `src/ui/shadertoywidget.cpp`
  - `src/gamewindow.h`
  - `src/gamewindow.cpp`
  - `docs/tasks/dream_status.md`
- 本轮新增文件：
  - 无
- 本轮返工内容：
  - 保留已有 STEP7A 外部 shader overlay 基础设施
  - 保留已有 STEP7B 场景绑定关系
  - 额外给外部 shader 增加 `iChannel0` 输入模式切换：
    - 可继续使用当前背景图 source texture
    - 也可改用内部 noise texture
  - `dream_1_fall` 继续使用 `dream_fall_xskgrw.frag`，默认走 source/fallback 路径
  - `dream_2_suspend` 与全部 `dream_3_future_*` / `dream_3_future_choice*` 继续使用 `dream_future_mtcgdf.frag`，但现在显式切到 noise texture 作为 `iChannel0`
- 返工原因：
  - 新的 future shader 依赖 `iChannel0` 作为噪声/程序纹理来源
  - 如果继续把外部 shader 一律喂给“当前背景图”，画面结果容易不稳定，也不贴合这版 shader 的预期用途
- 本轮未修改内容：
  - 未改梦境文本
  - 未改 7B 触发条件
  - 未改主线 7A、场景 8、场景 10/11/12
  - 未改 `AudioManager`
  - 未改梦境音频策略
  - 未改未来碎片变量系统
- 本轮验证：
  - 已核对当前两个 shader 文件均只保留一个 `mainImage(...)`
  - 已回查 `dream_1_fall -> dream_fall_xskgrw.frag`
  - 已回查 `dream_2_suspend` / `dream_3_future_*` / `dream_3_future_choice* -> dream_future_mtcgdf.frag`
  - 已确认 future shader 场景会调用新的 `setExternalChannel0UsesNoiseTexture(true)`
  - 已执行 `cmake --build build`，构建通过

## STEP8 Dream Audio 记录

- 执行日期：2026-05-11
- 本轮修改文件：
  - `data/audio_manifest.json`
  - `src/gamewindow.cpp`
  - `docs/tasks/dream_status.md`
- 本轮新增文件：
  - 无
- 本轮新增的音频 ID：
  - `bgm_dream_air_walk`
  - `amb_dream_air`
  - `amb_classroom_muffled`
  - `emo_dream_fall_low`
  - `sfx_dream_whoosh_soft`
  - `sfx_paper_fragment`
- 每个音频 ID 触发位置：
  - `dream_0_sleep`：继续使用 `amb_classroom_evening_and_hallway_loop`，并在 2000ms 内降到 20% 音量
  - `dream_1_fall`：播放 `emo_dream_fall_low`、`sfx_dream_whoosh_soft`，并额外播放一次 `sfx_paper_fragment`
  - `dream_2_suspend`：停止坠落段环境后，播放 `amb_dream_air`，并播放 `bgm_dream_air_walk`
  - `dream_3_future_choice*` / `dream_3_future_far` / `dream_3_future_together` / `dream_3_future_create`：保持 `amb_dream_air` 与 `bgm_dream_air_walk`
  - `dream_4_end`：保持 `amb_dream_air`，并把 `bgm_dream_air_walk` 压到约 70% 音量
  - `dream_5_wake`：播放 `sfx_bell`，停止梦境 BGM，并切回 `amb_classroom_evening_and_hallway_loop`
- 每个音频是否循环：
  - `bgm_dream_air_walk`：循环
  - `amb_dream_air`：循环
  - `emo_dream_fall_low`：不循环
  - `sfx_dream_whoosh_soft`：不循环
  - `sfx_paper_fragment`：不循环
  - `sfx_bell`：不循环
- 本轮淡入淡出时间：
  - `dream_0_sleep` 教室环境音量降低：2000ms
  - `dream_1_fall` 进入时环境音停止：1800ms
  - `dream_2_suspend` 的 `amb_dream_air`：fadeIn 1500ms
  - `dream_2_suspend` 的 `bgm_dream_air_walk`：fadeIn 2500ms
  - `dream_4_end` 的 BGM 压低：900ms
  - `dream_5_wake` 的梦境 BGM 停止：fadeOut 3000ms
- 缺失音频时如何 warning：
  - 继续复用 `AudioManager` 现有逻辑
  - 若 manifest 中找不到音频 ID，会输出 `[AudioManager] Unknown audio id ...`
  - 若文件缺失，会输出 `[AudioManager] Missing audio file ...`
  - 不会因为梦境音频缺失而中断场景推进
- 本轮的最小实现说明：
  - 没有修改 `AudioManager`
  - 单次 `SFX` 当前没有独立 fade-in 接口，所以 `dream_5_wake` 的铃声使用现有 one-shot 立即播放，这是对需求里“铃声渐入”的最小替代实现
  - `amb_classroom_muffled` 已加入 manifest，但当前 `dream_*` 实际实现仍优先复用教室环境音量压低方案，没有额外改成闷化版本
- 本轮未修改内容：
  - 未改梦境文本
  - 未改梦境 shader
  - 未改 7B 触发条件
  - 未改 `AudioManager` 实现
  - 未改主线 7A、场景 8、场景 10/11/12
- 本轮验证：
  - 执行 `cmake --build build`，构建通过
  - 回查确认 `dream_0_sleep`、`dream_1_fall`、`dream_2_suspend`、`dream_4_end`、`dream_5_wake` 已挂上对应音频逻辑
  - 回查确认新增音频 ID 已加入 `audio_manifest.json`
- 如何测试完整梦境音频：
  - 从正常流程进入 7B，满足休息条件后选择“睡一觉”
  - `dream_0_sleep`：确认教室环境音逐渐压低
  - `dream_1_fall`：确认有低频坠落感、whoosh 和纸张碎裂音
  - `dream_2_suspend`：确认梦境环境音与 `bgm_dream_air_walk` 开始淡入
  - `dream_3_future_choice*` 与三个未来碎片：确认梦境环境音和 BGM 持续保持
  - `dream_4_end`：确认 BGM 略微降低
  - `dream_5_wake`：确认铃声响起、梦境 BGM 退出、教室环境音恢复
- 下一步应执行：`STEP9_QA`

## STEP9 QA 记录

- 执行日期：2026-05-11
- 本轮修改文件：
  - `docs/tasks/dream_status.md`
- 本轮新增文件：
  - 无
- 本轮范围：
  - 只做 QA 检查，不修复实现，不新增功能
  - 采用静态代码/数据核对 + `cmake --build build` 构建验证
  - 本轮未做完整人工实机走读
- 检查结果：
  - 1. `rest_flag_scene1=false`、`rest_flag_scene5=false` 时，7B 选择睡觉不会进入梦境：通过
  - 2. `rest_flag_scene1=true`、`rest_flag_scene5=false` 时，7B 选择睡觉不会进入梦境：通过
  - 3. `rest_flag_scene1=true`、`rest_flag_scene5=true` 时，7B 选择睡觉会进入 `dream_0_sleep`：通过
  - 4. 梦境从 `dream_0_sleep` 到 `dream_5_wake` 的跳转链完整：通过
  - 5. `dream_5_wake` 结束后进入 `scene10_departure_bell`：通过
  - 6. `side_route_mode` 是否设置为 `dream`：未通过
  - 7. 主线 7A 是否不受影响：通过
  - 8. 7B 写作业路线是否不受影响：通过
  - 9. shader 关闭或失败时梦境是否仍可推进：通过
  - 10. 音频缺失时是否 warning 而非崩溃：通过
  - 11. 是否存在硬编码绝对路径：通过
  - 12. 是否存在未使用、重复或旧实现残留：存在残留风险
- 发现的问题：
  - `side_route_mode` 当前写入值仍然是 `"梦境"`，不是 STEP9 检查项要求的 `"dream"`。对应位置在 `data/story_main.json` 的 `dream_0_sleep` 写入，以及 `src/gamewindow.cpp` 里对 `side_route_mode == "梦境"` 的后续判断。
  - `STEP8` 中加入了 `amb_classroom_muffled`，但当前梦境实现并未实际使用它；`dream_0_sleep` 仍然复用 `amb_classroom_evening_and_hallway_loop` 并仅做音量压低。
  - `dream_5_wake` 当前使用的是 `sfx_bell` one-shot，不是单独的 `sfx_school_bell`，也不是严格意义上的“渐入铃声”；这是现有 `AudioManager` 能力下的最小替代实现。
  - 仓库里仍保留旧的 `scene7b_dream_*` 数据链和 `GameWindow` 对旧梦境前缀的部分特判；新 `dream_*` 链路已经接管入口，但旧实现尚未彻底收束，后续维护仍有混淆风险。
  - 路径上没有发现硬编码绝对路径，但仍存在相对搜索根与递归查找耦合：背景依赖 `../图` / `图` / `../../图`，shader 依赖 `assets/shaders` 下递归查找目标文件名。
- 本轮验证依据：
  - 用 `rg` 核对 `scene7b_sleep_attempt -> scene7b_sleep_check_scene5 -> dream_0_sleep` 条件链
  - 用 `rg` 核对 `dream_0_sleep -> ... -> dream_5_wake -> scene10_departure_bell` 跳转链
  - 用 `rg` 核对 `side_route_mode`、`dream_seen_*`、`dream_choice_state`、shader 失败回退、音频 ID 与路径搜索逻辑
  - 执行 `cmake --build build`，结果为通过
- 建议修复顺序：
  - 1. 统一 `side_route_mode` 的梦境值约定，决定继续沿用 `"梦境"` 还是整体切到 `"dream"`，避免 scene10+ 与运行时代码判断不一致。
  - 2. 清理或收束旧 `scene7b_dream_*` 残留逻辑，减少新旧梦境链并存带来的维护风险。
  - 3. 评估 `amb_classroom_muffled` 是否保留；若保留，就接入实际播放，否则从 manifest 和说明中去掉。
  - 4. 视需求决定是否要把 `dream_5_wake` 的铃声实现补齐为更接近策划要求的表现。
- 本轮未修改内容：
  - 未改 `data/story_main.json`
  - 未改 `src/gamewindow.cpp`
  - 未改主线 7A、场景 8、场景 10/11/12
  - 未改 `AudioManager`
  - 未改 `NarrativeEngine` 主流程
  - 未改 shader 与音频资源文件
- 下一步应执行：
  - 当前 `dream_status.md` 中已无新的 `todo` STEP；如需继续，建议先确认是否要新增“QA 问题修复”任务，再按新任务继续。
