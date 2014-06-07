#ifndef SCAN_H
#define SCAN_H

typedef struct declaration {
	declaration_specifier *dec_spec;
	list *init_declarator_list;
} declaration;

typedef struct declaratoin_specifier {
	int storage_class;
	int type_qualifier;
	void *type_specifier;
} declaration_specifier;

char *scan(FILE *stream);

void *parse_compound_stmt(FILE *stream);

#endif /* SCAN_H */