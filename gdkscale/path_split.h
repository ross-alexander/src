struct path_part_t {
  int32_t start, length, slash;
};

struct path_ext_t {
  char *dir;
  char *file;
  char *ext;
};

extern struct path_ext_t* path_split(const char *path);
