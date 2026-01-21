# How to contribute

I'm really glad you're reading this. You are probably much further into this than I am.

_This is very much work in progress._

In the meantime, look at the [issue templates](.github/ISSUE_TEMPLATE/).

## Planned tasks

- [x] Subcommand system, with `qdra xml` replacing current `main()` (_v0.4.0_)
- [x] Use `yaml_name` in `tax_parameters`' attributes (_v0.5.0_)
- [ ] Initialization
  - [ ] Questionnaire config for filling out `~/.quick_dra.yaml`
  - [ ] Add `qdra insurer` updating/creating `"płatnik"` and `"paramtery"`
  - [ ] Add `qdra insured` updating `"ubezpieczeni"` sequence
- [ ] Create a `qdra_gui`.
  - [ ] Obviously cross-platform
  - [ ] Using ElectronJS or Chromium's CEF is overkill
  - [ ] If not looking native, stick to current-ish Material Design
  - [ ] Support for editing of `~/.quick_dra.yaml`
  - [ ] A button to save the XML
- [ ] Automatic upload to ZUS ePłatnik

Thanks,
Marcin
