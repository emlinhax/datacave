```cpp
struct player_t
{
    u64 health;
};

int main()
{
    datacave::initialize();

    player_t* player = (player_t*)datacave::allocate_memory(sizeof(player_t));
    player->health = 100;

    while (true)
    {
        datacave::lock_all();

        Sleep(1000);
        printf("player_health: %d  @  0x%p\n", player->health, &player->health);
    }
}
```
