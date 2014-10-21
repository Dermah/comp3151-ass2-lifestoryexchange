inline announceDeath() {
   printf("%d dies blaming the mushroom soup.\n", id+1);
   goto end;
}

inline announceExchange () {
   printf("%d exchanges life stories with %d.\n", id+1, pairedWith+1);
}

inline announceVegetation () {
   printf("%d has a seniors’ moment.\n", id+1);
   goto end;
}