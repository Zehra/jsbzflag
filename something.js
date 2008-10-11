
events.getPlayerSpawnPos.add(function (e) {
    e.pos[2] += Math.random()*10;
    e.rot = 0;
});

