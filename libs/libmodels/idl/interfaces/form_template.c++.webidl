partial interface templates {
    [throws] static templates? parse_yaml([in] path path);
    [mutable] bool validate();
};

partial interface report_section {
    [mutable] bool validate();
};
