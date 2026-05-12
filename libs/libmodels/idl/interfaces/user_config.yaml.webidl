[postprocess, versioned=wersja, schema="https://raw.githubusercontent.com/mbits-os/quick_dra/refs/heads/main/data/schemas/user_config_schema.yaml"]
interface config {
    [yaml_name=wersja] attribute unsigned short version;
    [yaml_name="płatnik"] attribute payer_t payer;
    [yaml_name=ubezpieczeni] attribute sequence<insured_t> insured;
    [since_ver=2, yaml_name=wypadkowe, nullable, key=year_month]
    attribute record<DOMString, percent> accident_insurance;
};

[postprocess] interface payer_t: person {
    [yaml_name=nip] attribute string tax_id;
    [yaml_name=pesel] attribute string social_id;
};

[postprocess] interface insured_t: person {
    [yaml_name="tytuł ubezpieczenia"] attribute insurance_title title;
    [yaml_name=pesel] attribute string? social_id;
    [since_ver=2, yaml_name=historia, key=year_month] attribute record<DOMString, employment_history> history;
    [until_ver=1, yaml_name=wymiar] attribute ratio? part_time_scale;
    [until_ver=1, yaml_name=pensja] attribute currency? salary;
};

interface person {
    [yaml_name=nazwisko] attribute string last_name;
    [yaml_name="dowód"] attribute string? id_card;
    [yaml_name=paszport] attribute string? passport;
};

interface employment_history {
    [yaml_name=wymiar] attribute ratio? part_time_scale;
    [yaml_name=pensja] attribute currency? salary;
};
