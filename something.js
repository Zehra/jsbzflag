
events.getPlayerSpawnPos.add(function () {
    if (this.player.last_die_pos)
        this.pos = this.player.last_die_pos;
    this.player.sendMessage("spawning");
});

events.playerDie.add(function() {
    this.player.last_die_pos = this.pos;
});

slash_commands.gimme = function(flag) {
    print(flag);
    //this.from.removeFlag();
    print(this.from.giveFlag(flag, true));
}

