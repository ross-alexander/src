class thumbnail_t {
private:
public:
  std::map<std::string, int> dir_table;
  std::map<std::string, image_t*> image_table;
  std::string dst_dir;
  thumbnail_t();
  int dir_add(std::string);
  int dir_scan();
  void dst_set(std::string);
  void scale(int);
  void validate();
};
