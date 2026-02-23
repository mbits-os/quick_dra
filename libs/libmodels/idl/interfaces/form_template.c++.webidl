partial interface templates {
    [throws] static templates? parse_yaml([in] path path);
    bool validate();
};

partial interface report_section {
    bool validate();
};
