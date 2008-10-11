
function bz_GetPlayerSpawnPosEvent(e) {
    print(typeof e.pos);
    print(e.pos);
    e.pos[2] += Math.random()*10;
    e.handled = True;
}

