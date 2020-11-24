#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
using namespace std;
using namespace std::chrono;

#define BOARD_SIZE 16

#define PARR_LEN ((BOARD_SIZE * BOARD_SIZE + sizeof(uint64_t) * 8 - 1) / (sizeof(uint64_t) * 8))

#define posY(x) ((x) / BOARD_SIZE)
#define posX(x) ((x) % BOARD_SIZE)

#define MAP_X_O(x) (uint8_t)((x) % (sizeof(uint64_t) * 8))
#define MAP_Y_O(x) (uint8_t)((x) / (sizeof(uint64_t) * 8))

#define GET_POS_ID(id) (bitMap[(id)].num)
#define GET_POS_BIT(id) (bitMap[(id)].bit)

struct positionMap_S {
    uint64_t positionMap[PARR_LEN];
};

struct bitMap_S {
    uint8_t num;
    uint64_t bit;
};

typedef struct bitMap_S bitMap_S;

bitMap_S bitMap[BOARD_SIZE * BOARD_SIZE];

typedef struct positionMap_S positionMap_S;

positionMap_S positionMap[BOARD_SIZE * BOARD_SIZE];
positionMap_S solvedPositions[BOARD_SIZE];

atomic_uint64_t position = 0;

inline positionMap_S positionMaps_OR(positionMap_S map1, positionMap_S map2)
{
    positionMap_S map;
    for (int i = 0; i < PARR_LEN; i++) {
        map.positionMap[i] = map1.positionMap[i] | map2.positionMap[i];
    }
    return map;
}

positionMap_S getPositionMap(int position) {
    positionMap_S pmap;
    int i;
    int X = posX(position);
    int Y = posY(position);
    int posx, posy;

    for (i = 0; i < PARR_LEN; i++) {
        pmap.positionMap[i] = 0;
    }

    for (i = 0; i < BOARD_SIZE; i++) {
        pmap.positionMap[MAP_Y_O((Y * BOARD_SIZE) + i)] |= ((uint64_t)1 << MAP_X_O((Y * BOARD_SIZE) + i));
    }

    for (i = 0; i < BOARD_SIZE; i++) {
        pmap.positionMap[MAP_Y_O(X + i * BOARD_SIZE)] |= ((uint64_t)1 << MAP_X_O(X + i * BOARD_SIZE));
    }

    posx = X;
    posy = Y;

    while (posx < BOARD_SIZE && posy < BOARD_SIZE) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy++;
        posx++;
    }

    posx = X;
    posy = Y;

    while (posx >= 0 && posy >= 0) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy--;
        posx--;
    }

    posx = X;
    posy = Y;

    while (posx < BOARD_SIZE && posy >= 0) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy--;
        posx++;
    }

    posx = X;
    posy = Y;

    while (posx >= 0 && posy < BOARD_SIZE) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy++;
        posx--;
    }

    return pmap;
}

void fillBitMap() {
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int bitNum = x + y * BOARD_SIZE;
            bitMap[bitNum].num = MAP_Y_O(bitNum);
            bitMap[bitNum].bit = ((uint64_t)1 << MAP_X_O(bitNum));
        }
    }
}

void calcValidQpos(positionMap_S map, uint8_t N, uint8_t pos) {
    int endPos, posNum;
    if (N) {
        posNum = N * BOARD_SIZE;
        endPos = posNum + BOARD_SIZE;
    }
    else {
        posNum = pos + N * BOARD_SIZE;
        endPos = posNum + 1;
    }

    for (; posNum < endPos; posNum++) {
        if (map.positionMap[GET_POS_ID(posNum)] & GET_POS_BIT(posNum)) {
            continue;
        }
        else {
            if (N == (BOARD_SIZE - 1)) {
                position++;
                return;
            }
            else {
                solvedPositions[pos].positionMap[GET_POS_ID(posNum)] |= GET_POS_BIT(posNum);
                calcValidQpos(positionMaps_OR(map, positionMap[posNum]), N + 1, pos);
                solvedPositions[pos].positionMap[GET_POS_ID(posNum)] &= ~(GET_POS_BIT(posNum));
            }
        }
    }
}

void calcValidQpos_Thread_Func(int pos) {
    positionMap_S pmap;
    for (int i = 0; i < PARR_LEN; i++) {
        pmap.positionMap[i] = 0;
    }
    calcValidQpos(pmap, 0, pos);
}

int main()
{
    thread calcValidQpos_Thread[BOARD_SIZE];
    uint64_t start_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    fillBitMap();

    for (int i = 0; i < (BOARD_SIZE * BOARD_SIZE); i++) {
        positionMap[i] = getPositionMap(i);
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        calcValidQpos_Thread[i] = thread(calcValidQpos_Thread_Func, i);
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        calcValidQpos_Thread[i].join();
    }

    uint64_t stop_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    cout << "For board " << BOARD_SIZE << "x" << BOARD_SIZE << " calculation duration is " << stop_ms - start_ms << "ms\n";
    cout << "Detected " << position << " valid positions.\n";
}



/*
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
using namespace std;
using namespace std::chrono;

#define BOARD_SIZE 16

#define PARR_LEN ((BOARD_SIZE * BOARD_SIZE + sizeof(uint64_t) * 8 - 1) / (sizeof(uint64_t) * 8))

#define posY(x) ((x) / BOARD_SIZE)
#define posX(x) ((x) % BOARD_SIZE)

#define MAP_X_O(x) ((x) % (sizeof(uint64_t) * 8))
#define MAP_Y_O(x) ((x) / (sizeof(uint64_t) * 8))

struct positionMap_S {
    uint64_t positionMap[PARR_LEN];
};

struct bitMap_S {
    uint8_t num;
    uint64_t bit;
};

typedef struct positionMap_S positionMap_S;

positionMap_S positionMap[BOARD_SIZE * BOARD_SIZE];
positionMap_S solvedPositions[BOARD_SIZE];

atomic_uint64_t position = 0;

inline positionMap_S positionMaps_OR(positionMap_S map1, positionMap_S map2)
{
    positionMap_S map;
    for (int i = 0; i < PARR_LEN; i++) {
        map.positionMap[i] = map1.positionMap[i] | map2.positionMap[i];
    }
    return map;
}

positionMap_S getPositionMap(int position) {
    positionMap_S pmap;
    int i;
    int X = posX(position);
    int Y = posY(position);
    int posx, posy;

    for (i = 0; i < PARR_LEN; i++) {
        pmap.positionMap[i] = 0;
    }

    for (i = 0; i < BOARD_SIZE; i++) {
        pmap.positionMap[MAP_Y_O((Y * BOARD_SIZE) + i)] |= ((uint64_t)1 << MAP_X_O((Y * BOARD_SIZE) + i));
    }

    for (i = 0; i < BOARD_SIZE; i++) {
        pmap.positionMap[MAP_Y_O(X + i * BOARD_SIZE)] |= ((uint64_t)1 << MAP_X_O(X + i * BOARD_SIZE));
    }

    posx = X;
    posy = Y;

    while (posx < BOARD_SIZE && posy < BOARD_SIZE) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy++;
        posx++;
    }

    posx = X;
    posy = Y;

    while (posx >= 0 && posy >= 0) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy--;
        posx--;
    }

    posx = X;
    posy = Y;

    while (posx < BOARD_SIZE && posy >= 0) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy--;
        posx++;
    }

    posx = X;
    posy = Y;

    while (posx >= 0 && posy < BOARD_SIZE) {
        i = posx + posy * BOARD_SIZE;
        pmap.positionMap[MAP_Y_O(i)] |= ((uint64_t)1 << MAP_X_O(i));
        posy++;
        posx--;
    }

    return pmap;
}

void calcValidQpos(positionMap_S map, int N, int pos) {
    int endPos, posNum;
    if (N) {
        posNum = N * BOARD_SIZE;
        endPos = posNum + BOARD_SIZE;
    }
    else {
        posNum = pos + N * BOARD_SIZE;
        endPos = posNum + 1;
    }

    for (; posNum < endPos; posNum++) {
        if (map.positionMap[MAP_Y_O(posNum)] & ((uint64_t)1 << MAP_X_O(posNum))) {
            continue;
        }
        else {
            if (N == (BOARD_SIZE - 1)) {
                position++;
                return;
            }
            else {
                solvedPositions[pos].positionMap[MAP_Y_O(posNum)] |= ((uint64_t)1 << MAP_X_O(posNum));
                calcValidQpos(positionMaps_OR(map, positionMap[posNum]), N + 1, pos);
                solvedPositions[pos].positionMap[MAP_Y_O(posNum)] &= ~((uint64_t)1 << MAP_X_O(posNum));
            }
        }
    }
}

void calcValidQpos_Thread_Func(int pos) {
    positionMap_S pmap;
    for (int i = 0; i < PARR_LEN; i++) {
        pmap.positionMap[i] = 0;
    }
    calcValidQpos(pmap, 0, pos);
}

int main()
{
    thread calcValidQpos_Thread[BOARD_SIZE];
    uint64_t start_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    for (int i = 0; i < (BOARD_SIZE * BOARD_SIZE); i++) {
        positionMap[i] = getPositionMap(i);
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        calcValidQpos_Thread[i] = thread(calcValidQpos_Thread_Func, i);
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        calcValidQpos_Thread[i].join();
    }

    uint64_t stop_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    cout << "For board " << BOARD_SIZE << "x" << BOARD_SIZE << " calculation duration is " << stop_ms - start_ms << "ms\n";
    cout << "Detected " << position << " valid positions.\n";
}
*/