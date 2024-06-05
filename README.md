# datacave
hide data in no_access memory pages. when accessed internally, the data will temporarily become visible. \
this is useful against external memory reads (cheats, stealers, etc). \
the data will be hidden again after a very short time frame. \
additionally, the data is not just hidden but also encrypted (simple xor) 

# drawbacks
this is probably a heavy hit on performance. \
make sure to only utilize this where it makes sense. \
try to avoid high-frequency accesses.

# example:
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

this will make sure all caves are encrypted and hidden again. this pretty much also controls how big of a performance hit you want. \
make should be called periodically (example: at the end of a tick, every 100 milliseconds, etc...). \
calling it less often will result in less exceptions that have to be handled (aka. less cpu usage).
```cpp
datacave::lock_all();
```

