void* test();

int main(int argc, char const* argv[]) {
  int* i;
  *i = test();
  printf("%d\n", *i);
  return 0;
}

void* test() {
  x = 4;
  *xp = &x;
  return xp;
}
