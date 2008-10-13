// This plugin makes the player spawn at the same position that he/she last died.

events.playerDie.add(function() {
    // When a player dies, we save where it was that he died.
    // The player objects are persistent, so we can save our info there.
    this.player.last_die_pos = this.pos;
});

events.getPlayerSpawnPos.add(function () {
    // last_die_pos will be undefined if this is the first time the player 
    // spawns, so we check before using it.
    if (this.player.last_die_pos)
        this.pos = this.player.last_die_pos;
});


