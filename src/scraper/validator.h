#ifndef VALIDATOR_H
#define VALIDATOR_H

int is_day(const char *str);
int is_valid_number_nbsp(const char *s);
int is_valid_date_format(const char *str, char **out);
int is_valid_hour_format(const char *str);
int is_valid_class_format(const char *str);
int is_wrapped_in_parentheses(const char *s);

#endif // !VALIDATOR_H
