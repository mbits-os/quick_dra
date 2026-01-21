# Roadmap

- [x] Subcommand system, with `qdra xml` replacing current `main()` (_v0.4.0_)
- [x] Use `yaml_name` in `tax_parameters`' attributes (_v0.5.0_)
- [ ] Extract YAML parser to submodule independent of libmodels.
- [ ] Initialization
  - [ ] Questionnaire config for filling out `~/.quick_dra.yaml`
  - [ ] Add `qdra insurer` updating/creating `"płatnik"` and `"paramtery"`
  - [ ] Add `qdra insured` updating `"ubezpieczeni"` sequence
- [ ] Create a `qdra_gui`.
  - [ ] Support for editing of `~/.quick_dra.yaml`
  - [ ] A button to save the XML
  - _Notes:_
    - It must be cross-platform
    - Using ElectronJS or Chromium's CEF is overkill
    - If not looking native, stick to current-ish Material Design
- [ ] Automatic upload to ZUS ePłatnik
