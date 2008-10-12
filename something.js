events.getPlayerSpawnPos.add(function () {
    this.player.sendMessage("Air Spawn!" + this.player.currentFlag);
    this.pos[2] += Math.random()*10;
});

slash_commands.doit = function(word) {
    this.from.sendMessage(word);
    this.from.sendMessage(this.message);
}

