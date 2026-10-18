# Highlander Counter

Counter is an original Anathema technique informed by the sword-counter
concept in TSOB, not a copy of its implementation or Quickening system.

## Player command

`counter` requires Highlander Defense 4 and Powers 4, a legal active opponent,
and an owned forged katana equipped in the weapon or hold slot. The weapon
must be a slashing Highlander weapon with the player's matching quest owner.
An ordinary quested sword, somebody else's katana or a carried weapon does
not qualify.

- Activation: 1,000 mana and one second of command lag.
- Guard: two seconds, consumed by the first eligible incoming melee attack,
  whether its roll succeeds or fails.
- Success: 5% per Defense rank, capped at rank 6 (20%, 25%, 30%).
- Reuse: six seconds from activation, not reset by cancellation.
- A successful guard blocks that swing and readies one riposte for two seconds.
- The next normal equipped katana strike against the current legal opponent
  consumes the riposte and bypasses ordinary avoidance and Counter.
- Native damage calculation, caps, soak, sanctuary, immunities and safety
  still apply. It is not an execution, guaranteed HP loss or an extra attack.

`counter off` cancels the guard and riposte without refunding mana or
clearing recovery time. Missing ranks, weapon, mana or target reject before
the resource charge. Native safety rules apply, including their existing
treatment of an already ongoing fight.

Incoming fists, claws, fangs, kicks, horns, tails, weapon strikes and other
registered contact techniques qualify even when the attacker is unarmed.
The reply still requires the Highlander's own equipped katana. Punch, Bash,
Crippling Kick and Stun Fist check the guard before their contact effects,
so a successful parry also blocks their stun/lag/resource-drain effects;
the attacker still spends the normal attempted-action cost and recovery.
Spells, projectiles, breath, electrical surge and reflected shield/quill
damage do not consume the guard. Blindness/visibility, incapacitation,
loss of the katana, timeout and class/fight state are rechecked before use.
The timers run once per main-loop pulse, not once per descriptor or attack.

## Why the riposte is deferred

There is no nested call to `one_hit`, `damage` or a death callback. The reply
uses the next ordinary native strike and cannot arm another counter. It
keeps no target pointer across pulses. This avoids counter chains and does
not introduce NPC extraction in the middle of an attacker's callback.

Three transient `CHAR_DATA` timers track guard, riposte and reuse; no saved
field, class ID, rank cap, skill slot or resource pool is added. Reconnect
and copyover reset these short timers, as they do other transient combat
timers. They are not a crash-persistent cooldown policy.

## Verification and limits

The real-engine `tests/test_highlander_counter.c` covers class/rank/weapon/
owner/resource/target rejection, interpreter dispatch, exact charges and
lag, cooldown/cancellation, link-independent main-loop expiry, class/reset
cleanup, both RNG outcomes, non-weapon exclusions, protected targets,
disarming, and a native riposte that cannot trigger a prepared counter.
The suite also covers the registered natural/contact hit labels and native
Punch, Bash, Crippling Kick and Stun Fist calls. A closed contact-label
registry keeps legacy `TYPE_HIT` spells/projectiles from being misclassified;
new melee call sites must register there or use the contact preflight.

Success probabilities are design parameters, not multiplayer balance proof.
The ten-fight Highlander follow-up is an exploratory fixture comparison;
it must not replace the frozen all-class ranking or be described as a
statistically reliable win rate.

The all-melee version was exercised in ten paired fixed-seed matchups
(20 native ASan/UBSan runs). Control: 3 wins, 6 losses, 1 timeout draw.
Counter-prioritizing policy: 3 wins, 5 losses, 2 timeout draws, still a
30% win rate over all ten appearances. The Vampire loss became a 600-second
timeout draw; no matchup became a new win. These are policy-package
comparisons, not isolated effect estimates: Counter replaces a ready
Bash/Bladeflash choice and changes timing and RNG consumption.

Werewolf and Dragon used natural/claw forms; Monk used unarmed combat.
Their weapon/hold slots were left empty instead of forcing quested swords.
Remaining armor slots used class gear or maximum legal quested fillers.
The runner asserts those empty hand slots and verifies actual Werewolf
claw/fang damage in the native transcript. Across the Counter runs, native
logs recorded 108 successful guards and 95 ripostes, including guards
against unarmed/natural opponents. Trials with technical failures or illegal
command rejections would fail the runner rather than count as combat losses.
