// This plugin makes the player spawn at the same position that he/she last died.

events.getPlayerSpawnPos.add(function () {
    if (this.player.last_die_pos)
        this.pos = this.player.last_die_pos;
});

events.playerDie.add(function() {
    this.player.last_die_pos = this.pos;
});

