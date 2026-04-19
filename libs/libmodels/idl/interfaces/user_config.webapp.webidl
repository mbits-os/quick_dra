interface config {
    attribute payer_t? payer;
    attribute sequence<insured_t>? insured;
};

interface payer_t: person {
    attribute string? tax_id;
    attribute string? social_id;
};

interface insured_t: person {
    attribute insurance_title_t? title;
    attribute string? social_id;
    attribute ratio_t? part_time_scale;
    attribute currency? salary;
};

interface person {
    attribute string? first_name;
    attribute string? last_name;
    attribute string? kind;
    attribute string? document;
};

[peer_type="insurance_title"]
interface insurance_title_t {
	attribute string title_code;
	attribute unsigned short pension_right;
	attribute unsigned short disability_level;
};

[peer_type="ratio"]
interface ratio_t {
    attribute unsigned short num;
    attribute unsigned short den;
};
