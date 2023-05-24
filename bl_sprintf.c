
//Вспомогательная функция - добавляет число num к строке str и возвращает насколько изменилась длина
int add_int_to_str(int num, char *str)
{
	int i = 0;
	int len = 0;
	char reversed_str[256];

	if (num == 0)
	{
		reversed_str[i++] = '0';
		len++;
	}
	else
	{
		if (num < 0)
		{
			str[len++] = '-';
			num = -num;
		}

		while (num > 0)
		{
			reversed_str[i++] = num % 10 + '0';
			num /= 10;
		}

		while (--i >= 0)
		{
			str[len++] = reversed_str[i];
		}
	}

	return len;
}

//Вспомогательная функция - добавляет строку src к строке dest и возвращает насколько изменилась длина
int add_str_to_str(char *src, char *dest)
{

	int i = 0;

	while (src[i] != '\0')
	{
		dest[i] = src[i];
		i++;
	}

	return i;
}

//Вспомогательная функция - добавляет число num к строке str в шестнадцатеричном виде и возвращает насколько изменилась длина
int add_hex_to_str(unsigned int num, char *str) {
    int index = 0;

    if (num == 0)
        str[index++] = '0';

    while (num != 0)
    {
        int digit = num % 16;
        if (digit < 10)
            str[index++] = digit + '0';
        else
            str[index++] = digit - 10 + 'a';
        num /= 16;
    }

    for (int i = 0; i < index / 2; i++)
    {
        char temp = str[i];
        str[i] = str[index - i - 1];
        str[index - i - 1] = temp;
    }

    return index;
}

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#define va_list __builtin_va_list

int bl_sprintf(char *str, const char *format, ...)
{
	va_list arg_list;
	int i = 0;
	int len = 0;
	int arg_int;
	char arg_char;
	char *arg_str;

	va_start(arg_list, format);

	while (format[i] != '\0')
	{
		if (len >= 255)
			break;
		if (format[i] == '%')
		{
			switch (format[++i])
			{
			case 'd':
				arg_int = va_arg(arg_list, int);
				len += add_int_to_str(arg_int, str + len);
				break;

			case 'c':
				arg_char = (char)va_arg(arg_list, int);
				str[len++] = arg_char;
				break;

			case 's':
				arg_str = va_arg(arg_list, char *);
				len += add_str_to_str(arg_str, str + len);
				break;
			case 'x':
				arg_int = va_arg(arg_list, int);
				len += add_hex_to_str(arg_int, str + len);
				break;
			default:
				break;
			}
		}
		else
		{
			str[len++] = format[i];
		}

		i++;
	}

	va_end(arg_list);

	str[len] = '\0';
	return len;
}

