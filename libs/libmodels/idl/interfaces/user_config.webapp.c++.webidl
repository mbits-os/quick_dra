[quote="enum class emit { yaml, json };"] 
partial interface config {
    [throws, mutable] string store([default="emit::json"] emit syntax);
};
