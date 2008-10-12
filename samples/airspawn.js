// This plugin spawns the player up in the air at a random height.
events.getPlayerSpawnPos.add(function () {
    this.pos[2] += Math.random()*10;
});

