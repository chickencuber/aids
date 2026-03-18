#include <stdio.h>
#include "aids.h"


BaseClass(Animal, {
    int name;
    METHOD(void, speak, Animal* self); 
}) 
void Animal_speak(Animal* self) {
    printf("the animal %d spoke\n", self->name);
}
constructor(Animal, int name) {
    self->name = name;
    self->speak=Animal_speak;
}

Class(Dog, Animal, {
    int bark_volume;
    METHOD(void, bark, Dog* self); 
})

void Dog_speak(Dog* self) {
    printf("the dog %d spoke\n", self->name);
}
void Dog_bark(Dog* self) {
    printf("the dog %d barked with volume %d\n", self->name, self->bark_volume);
}
constructor(Dog, int name, int bark_volume){
    SUPER(Animal, name);
    self->bark_volume = bark_volume; 
    self->bark=Dog_bark;
    self->speak=Dog_speak;
}


void make_animal_speak(Animal* animal) {
    animal->speak(animal);
}

int main() {
    Dog* dog = new_heap(&DEFAULT_ALLOCATOR, Dog, 2, 5);
    dog->bark(dog);
    make_animal_speak(dog);
    FREE(&DEFAULT_ALLOCATOR, dog);
}
