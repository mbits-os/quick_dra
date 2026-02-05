# Changelog

All notable changes to this project will be documented in this file. See [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/) and [COMMITS.md](COMMITS.md) for commit guidelines.

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
