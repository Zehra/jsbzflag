print("something");
events.getPlayerSpawnPos.add(function (e) {
    e.player.sendMessage("Air Spawn!", e.player.currentFlag);
    e.pos[2] += Math.random()*10;
});
