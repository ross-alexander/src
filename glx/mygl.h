struct mygl {
  int mapped;
  int width;
  int height;
  void (*init)();
  void (*reshape)(int, int);
  void (*display)();
};

extern void init();
extern void reshape(int w, int h);
extern void display();

