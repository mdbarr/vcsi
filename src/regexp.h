void regexp_init(VCSI_CONTEXT vc);
VCSI_OBJECT regexpq(VCSI_CONTEXT vc, VCSI_OBJECT x);

VCSI_OBJECT regexp_compile(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT regexp_match(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT regexp_matches(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

VCSI_OBJECT regexp_compile_ci(VCSI_CONTEXT vc, VCSI_OBJECT x);
VCSI_OBJECT regexp_match_ci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);
VCSI_OBJECT regexp_matches_ci(VCSI_CONTEXT vc, VCSI_OBJECT x, VCSI_OBJECT y);

VCSI_OBJECT regexp_full(VCSI_CONTEXT vc,
			VCSI_OBJECT s,
			VCSI_OBJECT r);

VCSI_OBJECT REGEXP_MM[10];
