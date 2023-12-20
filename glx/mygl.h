struct mygl {
  int mapped;
  int width;
  int height;
  void (*init)();
  void (*reshape)(int, int);
  void (*display)();
};

