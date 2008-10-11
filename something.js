
events.getPlayerSpawnPos.add(function (e) {
    print(e.player.callsign, "spawning", e.player.another_value);
    e.pos[2] += Math.random()*10;
    e.rot = 0;
});

