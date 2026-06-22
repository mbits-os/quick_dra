# Changelog

All notable changes to this project will be documented in this file. See [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) and [COMMITS.md](COMMITS.md) for commit guidelines.

## [1.2.2](https://github.com/mbits-os/quick_dra/compare/v1.2.1...v1.2.2) (2026-06-22)

### Bug Fixes

- use `nowide::cin` in conversations ([92d2ead](https://github.com/mbits-os/quick_dra/commit/92d2ead98eeabda388a984ce5e440e60ae47e821)), fixes #78

### Continuous Integration

- redesign issue templates to use YAML ([7e3ec58](https://github.com/mbits-os/quick_dra/commit/7e3ec58c3b42d1f6cec668ee60eb1251f98f016c)), closes #79
- tweaks for Ubuntu 26.04 / gcc-15 ([7564ecc](https://github.com/mbits-os/quick_dra/commit/7564ecc80070ef7a1abcf0430ed190b0e006396a)), closes #83
- gather images created by failing tests ([a042b74](https://github.com/mbits-os/quick_dra/commit/a042b745fda8fce957003c3302e70fc20657f791))
- use proj-flow to set `$LC_ALL` for tests ([4ecd416](https://github.com/mbits-os/quick_dra/commit/4ecd416a4bfdbea17dc9efb7e0c446864475dba5)), closes #72

### Tests

- stabilize `PagesTest::mainWindow` test ([b167b4a](https://github.com/mbits-os/quick_dra/commit/b167b4aadbe4258249b0f5cc2ab3737bacca5424))
- stabilize `Globals.config` and `Globals.configModified` ([c2b334e](https://github.com/mbits-os/quick_dra/commit/c2b334e44ba0c7d6598b82770aee8e30fba8d8bd))
- stabilize `ControlsTest::PanelButtonGroup_layout` for Ubuntu 26.04 ([0459339](https://github.com/mbits-os/quick_dra/commit/045933917ad2eaaa3869b5073145dbacaab72c8e))
- stabilize `GuiTest::PageHeader_centeringTitle` test ([9f9e4e0](https://github.com/mbits-os/quick_dra/commit/9f9e4e04e233086610869f8bd616196e0bf0f830))
- gather test results from QTest-based runs ([c17cb0c](https://github.com/mbits-os/quick_dra/commit/c17cb0c7107e7470bf299fc15ae7f84e291cd229))
- convert Qt XMLs to JUnit with a tool ([0826bb3](https://github.com/mbits-os/quick_dra/commit/0826bb3513832c9059a7ea61ae07eb30ba98679c))
- unify control checks ([d52ff50](https://github.com/mbits-os/quick_dra/commit/d52ff50c5de0328989db37555e6b3f502503d6f4))
- preselect pl_PL in QLocale ([a0b21de](https://github.com/mbits-os/quick_dra/commit/a0b21deccd6339cfc24a16d04e821419f15a7c0c))

## [1.2.1](https://github.com/mbits-os/quick_dra/compare/v1.2.0...v1.2.1) (2026-06-15)

### Bug Fixes

- expose insured and report button groups ([abafe01](https://github.com/mbits-os/quick_dra/commit/abafe0168705c157a61212a928237f54483fe347))
- enumerate buttons in a group ([d965914](https://github.com/mbits-os/quick_dra/commit/d9659147dbd4e3e4fa67fd5b4a7e2e8a8bc6aea1))
- reorganize mouse code, react to LButton only ([bbd88ee](https://github.com/mbits-os/quick_dra/commit/bbd88ee4e0c3d5f75e1d7b06f927d6924e1c42a4))
- allow SVG scaling ([5e1154d](https://github.com/mbits-os/quick_dra/commit/5e1154d2c3ef42028cdbb3d14ef9e71363bd507c))
- do not mix icon size API with `QWidget::size()` ([34fd250](https://github.com/mbits-os/quick_dra/commit/34fd250d095a4efb94d189c04ac1749052611ca7))
- flip size guards for `Glyph::setIconSize` ([35a136d](https://github.com/mbits-os/quick_dra/commit/35a136d5cf6a3ffaf6715b77533cf9b90695503e))
- remove unnecessary copy leading to use after free ([f787880](https://github.com/mbits-os/quick_dra/commit/f7878804f4b0f5014d99f5f276ac96b05065b101))
- be aware of globals initialization order ([f1abed0](https://github.com/mbits-os/quick_dra/commit/f1abed0c25fbae6a6ca50bad18a38e66fc07b85d))
- cleanup and general test friendliness ([33288b5](https://github.com/mbits-os/quick_dra/commit/33288b5c3b8d92972156bf995656706d5a7f34e6))
- add skipping GitHub as an option in GUI code ([392309d](https://github.com/mbits-os/quick_dra/commit/392309da66f5cd09a912d16ddd773cf251c2a5da))
- differentiate payer and insured, when name is missing ([a0202a6](https://github.com/mbits-os/quick_dra/commit/a0202a6494ac3c06757abcc64621d034a5b11af3))
- clean up LaidOut code ([3fd7f2f](https://github.com/mbits-os/quick_dra/commit/3fd7f2f310fdbe9fd2ed4aa4874f5e8b0284edb1))
- skip downloading tax config from GitHub as needed ([f83db73](https://github.com/mbits-os/quick_dra/commit/f83db73060bd6e26f3807e88a401900287d7664d))
- allow overriding config data directory ([2ca8713](https://github.com/mbits-os/quick_dra/commit/2ca8713c43045342a21e7d79f9d5bd8bead9560f))
- don't use `decltype` on lambda in template base classes ([341f374](https://github.com/mbits-os/quick_dra/commit/341f3746f1c64cc97ed56277186706cd600f8bba))

## [1.2.0](https://github.com/mbits-os/quick_dra/compare/v1.1.0...v1.2.0) (2026-06-06)

### New Features

- use `<clocale>` for number formatting ([05c5354](https://github.com/mbits-os/quick_dra/commit/05c5354f132888b012a1c3b84c8027f7f79d36a3))
- modify payer and insured list ([8207cc7](https://github.com/mbits-os/quick_dra/commit/8207cc766e93a92f0220ccbd726ebaa1cb6cc04d))
- save KEDU for current config ([7743fd6](https://github.com/mbits-os/quick_dra/commit/7743fd68d5a45fcd4e6bcbe381f33e6cfe1151c6))
- edit report id ([dc935e9](https://github.com/mbits-os/quick_dra/commit/dc935e922a569662a6ec983147a00735b4c883d5))
- show filled-out RCA/DRA forms ([f95d621](https://github.com/mbits-os/quick_dra/commit/f95d621588b44ebba4c8f4f18d1e8df537699304))
- add Home page, with all buttons disabled ([56205d5](https://github.com/mbits-os/quick_dra/commit/56205d5c57001033f42ba992eef0f3514ec44c01))
- observe outside config changes ([973b63f](https://github.com/mbits-os/quick_dra/commit/973b63feee6f690b217b24a67cc75a1bf94142d2))
- restore window position ([4b22fc5](https://github.com/mbits-os/quick_dra/commit/4b22fc5f5044ee890ae3787e8cc67fd68b10cb72))
- add global data with page stack ([da2c993](https://github.com/mbits-os/quick_dra/commit/da2c993914be56735affe8f6f4e3ff62e78977bb))
- add empty Qt app ([71c1049](https://github.com/mbits-os/quick_dra/commit/71c1049781917871e141e263cb6e22b598ab7fdb))
- add `to_lower` ([fca46b7](https://github.com/mbits-os/quick_dra/commit/fca46b76cf21e44458aef79a5f18ece284eb5c22))
- add min_length/max_length to liblex  validators ([a7dc633](https://github.com/mbits-os/quick_dra/commit/a7dc63342eb644f713c0c6184824d79a6a8390fb))
- support graphical form presentation info ([0f5f493](https://github.com/mbits-os/quick_dra/commit/0f5f4934ab47c743385e5d6d8311888d8fd34b57))
- add year_month as first-class value type ([b49ec66](https://github.com/mbits-os/quick_dra/commit/b49ec666aba355807a9334240c5cc10694f72920))

### Bug Fixes

- clean GUI code warnings ([382b672](https://github.com/mbits-os/quick_dra/commit/382b6726544eaf9eb03a3c817fc00f7b77d132a1))
- downgrade to API from Qt 6.8.3 ([03f72bb](https://github.com/mbits-os/quick_dra/commit/03f72bbd19c168c804b73dad6a5bdd34002d158b))
- compile locale on msvc ([c618a10](https://github.com/mbits-os/quick_dra/commit/c618a105dfa4e590a46c7b48416f5d089507ec2d))
- remove unused function ([ee8d3f2](https://github.com/mbits-os/quick_dra/commit/ee8d3f2351a087db579921077e1455f835df18bc))
- issues uncovered during testing ([b383c33](https://github.com/mbits-os/quick_dra/commit/b383c33d5d4284ed822d4fbd3c63253561e8a840))
- allow rendering document fragments ([0537e87](https://github.com/mbits-os/quick_dra/commit/0537e8768f88bd76135d3ad7492791be15d73505))
- drop xml dependency on entire options object ([6d54762](https://github.com/mbits-os/quick_dra/commit/6d54762a71b43f785478c0c181f89ac5ec267430))
- extract tax_config loader ([f55a92f](https://github.com/mbits-os/quick_dra/commit/f55a92f6a6ad1d5760c8b6281d6f5f3c30e3115f))
- check if lookup found anything at all ([842fff6](https://github.com/mbits-os/quick_dra/commit/842fff64efd5fcb97c9614d3a2f916e1588db3bc))

## [1.1.0](https://github.com/mbits-os/quick_dra/compare/v1.0.0...v1.1.0) (2026-05-12)

### New Features

- add `qdra config` ([ae102f0](https://github.com/mbits-os/quick_dra/commit/ae102f037719e7d2212a2baa06d788ff80d40cf6))
- add $schema when writing user config ([7c66737](https://github.com/mbits-os/quick_dra/commit/7c66737f6e817795cfde56b98d02a31b333e182e))
- updating tests to config v2 ([fa93e5a](https://github.com/mbits-os/quick_dra/commit/fa93e5a0dad4347f1357ada293f91b90a3fccb69))
- create a insured history in user config v2 ([190539a](https://github.com/mbits-os/quick_dra/commit/190539a914bbe477169148885cbf4e847613080e))

### Bug Fixes

- give chrono.hpp more responsibilities ([2acd13f](https://github.com/mbits-os/quick_dra/commit/2acd13f796858406fa361b684b0da534e2894103))
- favor `wypadkowe` in user config over tax config ([87fe8d1](https://github.com/mbits-os/quick_dra/commit/87fe8d1100370e287ac9561da6d6167d52d5fd18))
- apply suggestions by PVS-Studio ([b000f0f](https://github.com/mbits-os/quick_dra/commit/b000f0f7ff47c7f0a1406d00973544ea7e69969a))

## [1.0.0](https://github.com/mbits-os/quick_dra/compare/v0.11.1...v1.0.0) (2026-04-13)

### New Features

- **breaking**: releasing version 1.0.0 of this project ([7a9fee3](https://github.com/mbits-os/quick_dra/commit/7a9fee342b441ea4b691fa8c4ffd76f69208ec32))

## [0.11.1](https://github.com/mbits-os/quick_dra/compare/v0.11.0...v0.11.1) (2026-04-13)

### Bug Fixes

- apply suggestion from PVS-Studio ([d394721](https://github.com/mbits-os/quick_dra/commit/d39472101611d6e04b7935289f1552cc0d33f1f6))
- **docs**: fix typos in usage.md ([29874ac](https://github.com/mbits-os/quick_dra/commit/29874ac37b486f09792264ae29da34da62d33c23))

## [0.11.0](https://github.com/mbits-os/quick_dra/compare/v0.10.0...v0.11.0) (2026-04-12)

### Bug Fixes

- **docs**: fix file name for `-m0` ([3308951](https://github.com/mbits-os/quick_dra/commit/3308951d31548dcaf19a14daf68fe835fd9a33b4))

### Build System

- extract `quick_dra.test_coverage` to its own repo ([e20586d](https://github.com/mbits-os/quick_dra/commit/e20586d5fa41b72f188a28613086d87b9e487a89))

### Continuous Integration

- enable clang-tidy in action ([468d15b](https://github.com/mbits-os/quick_dra/commit/468d15bcc99dc7de61348013af4fc000aeaf9450))
- move Flow bootstrapping to an action ([2cac4d4](https://github.com/mbits-os/quick_dra/commit/2cac4d4509bc912bef4120619e7d59e9fb1aeedd))

## [0.10.0](https://github.com/mbits-os/quick_dra/compare/v0.9.2...v0.10.0) (2026-04-10)

### New Features

- add payer to `list` output ([480f38a](https://github.com/mbits-os/quick_dra/commit/480f38a017810cb6a1e9b8a188f5ebcc56175372))

### Bug Fixes

- align help generator with shape of usage.md ([00de468](https://github.com/mbits-os/quick_dra/commit/00de468a896e15a23e46fad98648435818d337d8))
- clean warnings on GCC ([d61ecf8](https://github.com/mbits-os/quick_dra/commit/d61ecf81f33f99ff79a18d7ba3b0fed15887b995))
- apply clang-tidy suggestions ([4b89e10](https://github.com/mbits-os/quick_dra/commit/4b89e1007536afb0884bea4dd1c6f6c75b65ab06))
- apply PVS Studio suggestions ([5460953](https://github.com/mbits-os/quick_dra/commit/546095394afcf6d7e14804f2dfc5fb48a2710ba6))
- **docs**: build a full-er usage doc ([3738272](https://github.com/mbits-os/quick_dra/commit/3738272e74ce3148ae8188b78de4495f20afcb9c))

## [0.9.2](https://github.com/mbits-os/quick_dra/compare/v0.9.1...v0.9.2) (2026-02-28)

### Bug Fixes

- extract libcli for testability ([826efc8](https://github.com/mbits-os/quick_dra/commit/826efc87e15d42f0d126b9bec7f3358211b69e8f))
- make stdin injectable in conv library ([6602cad](https://github.com/mbits-os/quick_dra/commit/6602cad1b2036a88c09ebf02e3f0c2825840ae25))
- **docs**: add more usage to README ([c598e63](https://github.com/mbits-os/quick_dra/commit/c598e636bd6cf75756104271c4762bdde46f391a))

## [0.9.1](https://github.com/mbits-os/quick_dra/compare/v0.9.0...v0.9.1) (2026-02-23)

### Bug Fixes

- reset document props after use ([3b070be](https://github.com/mbits-os/quick_dra/commit/3b070bef397295c93f5691265a1f82b090cc945c))
- allow reading empty strings ([668bc60](https://github.com/mbits-os/quick_dra/commit/668bc60b63851a43124919b4549309906f0c0a7c))
- reject `--today 1999-11-31` as invalid date ([d24a5cd](https://github.com/mbits-os/quick_dra/commit/d24a5cd7c046f582a541d63a8d966c41ba8758a1))
- drop the idea of "partially-loaded" config, just check for errors ([7cd0bf2](https://github.com/mbits-os/quick_dra/commit/7cd0bf247b0a85ac544d2f3c0ef560d5802e14ab))
- remove idea of "fixing a checksum" ([93f1d1b](https://github.com/mbits-os/quick_dra/commit/93f1d1be33558797f8d7986d0d39954b34632dd5))
- patch corner case for some esoteric Windows setups ([857af32](https://github.com/mbits-os/quick_dra/commit/857af32896a3f40f027823ca42d20924bafb2d3f))
- gcc compilation warnings ([7d773e4](https://github.com/mbits-os/quick_dra/commit/7d773e4d908d8c72f1ab4d69293e9d3db1f145e3))
- remove compilation warnings on clang ([04cce7a](https://github.com/mbits-os/quick_dra/commit/04cce7a47f4095ea236d69fbbfb3660da31eada4))
- stricter version comparison for proj_flow package ([96cb596](https://github.com/mbits-os/quick_dra/commit/96cb5969efd26860f0770c211824609b691fadc4))
- **docs**: add Coveralls badge to readme ([b902f42](https://github.com/mbits-os/quick_dra/commit/b902f42e344e4f1e075ce80915e9bb575745cece))

## [0.9.0](https://github.com/mbits-os/quick_dra/compare/v0.8.0...v0.9.0) (2026-02-10)

### New Features

- provide support for `qdra insured edit` command ([76531ac](https://github.com/mbits-os/quick_dra/commit/76531acb4c6cd5816f7b5a31247782645b412110))

### Bug Fixes

- apply super-linter suggestions ([bffad00](https://github.com/mbits-os/quick_dra/commit/bffad0031013dd30c2a640bf10f9d8aaa9d06dd7))
- identify all the YAML schemas ([83db6ba](https://github.com/mbits-os/quick_dra/commit/83db6bab6dc450e518c7e6ea8ccf332e80e9eab6))
- simplify enum interactions ([34af7a6](https://github.com/mbits-os/quick_dra/commit/34af7a6e391f4764b7e0d61dcef5d98aa3c08f4b))
- move insured searching to libconv ([93fe443](https://github.com/mbits-os/quick_dra/commit/93fe443ff5d6bbee296fdaecf65115df372effdb))

## [0.8.0](https://github.com/mbits-os/quick_dra/compare/v0.7.3...v0.8.0) (2026-02-08)

### New Features

- provide support for `qdra insured list` command ([e9beab4](https://github.com/mbits-os/quick_dra/commit/e9beab4e36d786d1424cd09986c31899ea24f17e))
- provide support for `qdra insured remove` command ([32d29ff](https://github.com/mbits-os/quick_dra/commit/32d29ffa6af6c3712ebe5ba712c355de45e94385))
- provide support for `qdra insured add` command ([942d6c8](https://github.com/mbits-os/quick_dra/commit/942d6c895a1e3a1b1677f6ed3b4b093b8b89283e))
- enforce the conversation API with concepts ([71d9570](https://github.com/mbits-os/quick_dra/commit/71d95703dfe7669a77b0d0b14d4f91a03944c1bd))
- add machinery for document kind conversation ([3184c64](https://github.com/mbits-os/quick_dra/commit/3184c64a6a50bc875c31807f81e4008f9d279699))
- provide empty `qdra insured` sub-commands ([2e04714](https://github.com/mbits-os/quick_dra/commit/2e047147637f174d08ebaa5c8e506e399cb7f46c))

### Bug Fixes

- remove compilation issue with GCC13 on Ubuntu 22.04 ([35ef1bb](https://github.com/mbits-os/quick_dra/commit/35ef1bbe6d6e64b3a46fcf92c6a222bcc626df34))
- use config data when cli is missing, again ([f15bc43](https://github.com/mbits-os/quick_dra/commit/f15bc430d5f6a43570522fe2e53a878f7b386c9a))
- check required params, when `-y` is also present ([bf7e36e](https://github.com/mbits-os/quick_dra/commit/bf7e36eb9486059a6f2f37f7369dd0ac89116d49))
- separate ratio and insurance_title parsers ([02a3a73](https://github.com/mbits-os/quick_dra/commit/02a3a73e53dd1bfe2f60e86d0050aad7987da485))
- rename each `command.cpp` to be unique ([7849762](https://github.com/mbits-os/quick_dra/commit/78497623a9d15808c99d62be33290e8df3c675b1))
- extract common partial loading handling ([963cdd5](https://github.com/mbits-os/quick_dra/commit/963cdd564561c8171542c7e72bb6e8b6c1963783))
- always name custom config param, `--config` ([59816bd](https://github.com/mbits-os/quick_dra/commit/59816bd1c32c42fdb6f881800a78a959eebc5553))
- rename remuneration field what it is, a salary ([87eee5b](https://github.com/mbits-os/quick_dra/commit/87eee5bea0a9cf135c38c534dfba7e99e1dbe00e))
- decouple the subcommand code from root command ([0eeffac](https://github.com/mbits-os/quick_dra/commit/0eeffaca38e94a5c2eae9926960df827b8a16f22))
- hide warning from 3rd party library ([7039e51](https://github.com/mbits-os/quick_dra/commit/7039e51a7df40b1f7e5b511ca3f019c5a18a5776))

## [0.7.3](https://github.com/mbits-os/quick_dra/compare/v0.7.2...v0.7.3) (2026-02-05)

### Bug Fixes

- **docs**: add something about CLI to readme ([d95818f](https://github.com/mbits-os/quick_dra/commit/d95818f286e5d0ea5dc1723cbd7f91b050bd17e2))
- **docs**: fix wording on Super Linter ([7bd3fc6](https://github.com/mbits-os/quick_dra/commit/7bd3fc6e871f59e5e3c6e8d57fc087dc40d3d5d5))

## [0.7.2](https://github.com/mbits-os/quick_dra/compare/v0.7.1...v0.7.2) (2026-02-05)

### Bug Fixes

- better contribution calculation ([1f97d6d](https://github.com/mbits-os/quick_dra/commit/1f97d6d87e3e8f86ee3dcd26dd69fd5b0629379b))

## [0.7.1](https://github.com/mbits-os/quick_dra/compare/v0.7.0...v0.7.1) (2026-02-04)

### Bug Fixes

- do not react to borked YAML config in `payer` ([02db9be](https://github.com/mbits-os/quick_dra/commit/02db9bee17267a779995ebd10c8b4c4bf8c2fef4))
- add WinAPI-based implementation ([860cb1b](https://github.com/mbits-os/quick_dra/commit/860cb1b5b0c43986e2d0704dc608768d2b49689f))
- add ICU-based implementation ([5dced9d](https://github.com/mbits-os/quick_dra/commit/5dced9def739febc1328213834e2c4b4ed280213))
- prepare for new `to_upper` implementation(s) ([4d40c34](https://github.com/mbits-os/quick_dra/commit/4d40c346b6fab4dbcbfa70abab14b6c169802a6a))

## [0.7.0](https://github.com/mbits-os/quick_dra/compare/v0.6.2...v0.7.0) (2026-02-01)

### New Features

- add `qdra payer` sub-command ([37a02c8](https://github.com/mbits-os/quick_dra/commit/37a02c87713ff843a0ade473eab9d246903d02e8))
- add libconv to share with future commands ([8bea5ac](https://github.com/mbits-os/quick_dra/commit/8bea5ac85e3828135e3b82b27fe4ef3db9ee78f4))
- load partial user config ([05d82b7](https://github.com/mbits-os/quick_dra/commit/05d82b7a35ae1d5d907721db135c16d8dd33efd1))
- validate NIP, PESEL, ID card and PL passport numbers ([7c73438](https://github.com/mbits-os/quick_dra/commit/7c734384ea625110e84b3e6f40cb3df6b12ace2a))
- add `--today` to `qdra xml` ([f8d43dc](https://github.com/mbits-os/quick_dra/commit/f8d43dcf8398ab9ed7bd535e22937330712b1a24))

### Bug Fixes

- compile with g++-13 ([a3cdf91](https://github.com/mbits-os/quick_dra/commit/a3cdf91ee1142742a69a3fe11d7c889c1223b943))
- compile with g++-14 ([725af3e](https://github.com/mbits-os/quick_dra/commit/725af3ef8405ee6fe6e0ae2cfd53412f8600ea89))
- rename `insurer` to `payer` ([dea44f0](https://github.com/mbits-os/quick_dra/commit/dea44f01e8fec82c4e1deaaf6b2c90f4e7f94742))

## [0.6.2](https://github.com/mbits-os/quick_dra/compare/v0.6.1...v0.6.2) (2026-01-30)

### Bug Fixes

- allow overriding tax config from command line ([bc66c75](https://github.com/mbits-os/quick_dra/commit/bc66c75fffeb0ae8692ffafb07e62cb9c842f664))

## [0.6.1](https://github.com/mbits-os/quick_dra/compare/v0.6.0...v0.6.1) (2026-01-30)

### Bug Fixes

- properly round negative numbers ([0b0f3c3](https://github.com/mbits-os/quick_dra/commit/0b0f3c3c2413462a3925516ae8f6b1231d734d8c))
- allow PLN suffix for currency ([a63d332](https://github.com/mbits-os/quick_dra/commit/a63d332c8678d5c5b6390f979da0289881a0f821))

## [0.6.0](https://github.com/mbits-os/quick_dra/compare/v0.5.0...v0.6.0) (2026-01-30)

### New Features

- switch between tax parameter handling methods ([9ea864d](https://github.com/mbits-os/quick_dra/commit/9ea864d3946507c054ec60b858576315477d5bc5))
- read the tax configuration ([66503ca](https://github.com/mbits-os/quick_dra/commit/66503caa553731fc01bf5ad49f5d19aa627f34e9))
- add new tax config + $schemas ([89cfb54](https://github.com/mbits-os/quick_dra/commit/89cfb54b0ef3899b32f789a18a324ddafd56c4e0))

### Bug Fixes

- move basic types deeper into library layer cake ([98530bb](https://github.com/mbits-os/quick_dra/commit/98530bb6237dfed54b8382afa8a8cef69dc1e2bf))

## [0.5.0](https://github.com/mbits-os/quick_dra/compare/v0.4.0...v0.5.0) (2026-01-29)

### New Features

- extract independent YAML code to library ([d04a0db](https://github.com/mbits-os/quick_dra/commit/d04a0db5fcca7179a6fe0371bd3fd699c0f4d70f))
- use `yaml_name`s for all user-facing configs ([206deb7](https://github.com/mbits-os/quick_dra/commit/206deb76d05cfffec99a2f9082d66f90c0e46cd8))

### Bug Fixes

- allow artifact building ([33d1c66](https://github.com/mbits-os/quick_dra/commit/33d1c66d54dfbcee98f7fecf92818f8ec9e7c966))
- **docs**: prepare a better CONTRIBUTION doc ([4cbc7ab](https://github.com/mbits-os/quick_dra/commit/4cbc7ab8a1608a7376c61531909b68bbc37347ca))

## [0.4.0](https://github.com/mbits-os/quick_dra/compare/v0.3.0...v0.4.0) (2026-01-21)

### New Features

- introduce sub-command system to CLI app (#12) ([5e1a27d](https://github.com/mbits-os/quick_dra/commit/5e1a27dc74ebbfe60c9decabfe268b025d584ae0))

### Bug Fixes

- **docs**: fix wording in documentation ([c87080a](https://github.com/mbits-os/quick_dra/commit/c87080a5a993c05aa6a0d9ebb106fcf29aed0b27))
- **docs**: list planned tasks ([fcfdb1d](https://github.com/mbits-os/quick_dra/commit/fcfdb1df559d9802dae624694de7cb4081de36e6))

## [0.3.0](https://github.com/mbits-os/quick_dra/compare/v0.2.1...v0.3.0) (2026-01-19)

### New Features

- add payment summary info ([fb61a47](https://github.com/mbits-os/quick_dra/commit/fb61a476e068c80db2c25db9c4b41a37d5572ac8))

### Bug Fixes

- GCC has not ranges::to yet ([b78dfcd](https://github.com/mbits-os/quick_dra/commit/b78dfcd5bd39c8d6aa05c17052ac741c217b686f))
- remove old widl codegen ([1e01c70](https://github.com/mbits-os/quick_dra/commit/1e01c70462361a37b79333caf540be89a68094e4))
- use external webidl generator ([a18edbd](https://github.com/mbits-os/quick_dra/commit/a18edbd95451381afb25df6037b98cb109209991))

## [0.2.1](https://github.com/mbits-os/quick_dra/compare/v0.2.0...v0.2.1) (2026-01-09)

### Bug Fixes

- generate icons w/out need of masks ([6b5437f](https://github.com/mbits-os/quick_dra/commit/6b5437f31da9355b760b4d1821d33d63e88532fc))

## [0.2.0](https://github.com/mbits-os/quick_dra/compare/v0.1.0...v0.2.0) (2026-01-09)

### New Features

- allow pretty-printing XML ([e7f7ed0](https://github.com/mbits-os/quick_dra/commit/e7f7ed06afa8742f5f0fd73e9fb79f84c3f2902d))
- download github version of minimal pay for future-proofing ([45c600b](https://github.com/mbits-os/quick_dra/commit/45c600b7e6f74a5afa081cc8fa57a18975b6b686))
- add debug levels and normalize output ([c6961f0](https://github.com/mbits-os/quick_dra/commit/c6961f07c0edc7952260d9bbea1942011804382a))

### Bug Fixes

- clean clang compilation ([8c9c655](https://github.com/mbits-os/quick_dra/commit/8c9c655775b8c0c9891aa5676aedf46a83a58c7d))

## [0.1.0](https://github.com/mbits-os/quick_dra/commits/v0.1.0) (2026-01-08)

### New Features

- Initial commit ([fb02f29](https://github.com/mbits-os/quick_dra/commit/fb02f295d0ada405f15bd5b2a3199060e89ba46d))
