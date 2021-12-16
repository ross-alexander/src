static const char *str = "looper";

void loop(unsigned int *p)
{
  for (int i = 0; str[i]; i++)
    *p = str[i];
}
