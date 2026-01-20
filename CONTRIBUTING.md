# How to contribute

I'm really glad you're reading this. You are probably much further into this, than I am.

_This is very much work in progress._

In the meantime, look at the [issue templates](.github/ISSUE_TEMPLATE/).

## Planned tasks

- [ ] Subcommand system, with `qdra kedu` replacing current `main()`
- [ ] Use `yaml_name` in `tax_parameters`' attributes
- [ ] Initialization
  - [ ] Questionare config for filling out `~/.quick_dra.yaml`
  - [ ] Add `qdra insurer` updating/creating `"platnik"` and `"paramtery"`
  - [ ] Add `qdra insured` updating `"ubezpieczeni"` sequence
- [ ] Create a `qdra_gui`.
  - [ ] Obviously cross-platform
  - [ ] Using ElectronJS or Chromium's CEF is an overkill
  - [ ] If not looking native, stick to current-ish Matertial Design
  - [ ] Support for editing of `~/quick_dra.yaml`
  - [ ] A button to save the XML
- [ ] Automatic upload to ZUS ePłatnik

Thanks,
Marcin
