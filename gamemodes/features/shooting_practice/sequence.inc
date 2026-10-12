#if defined SP_SEQUENCE_INCLUDED
    #endinput
#endif
#define SP_SEQUENCE_INCLUDED

// Build once per attempt. Each configured position appears exactly once and the
// type counts are exact. Fixed distribution respects each position's saved type.
stock PracticeBuildSequence(zone, slots[PRACTICE_MAX_TARGETS], bool:noShoot[PRACTICE_MAX_TARGETS], models[PRACTICE_MAX_TARGETS])
{
    new total;
    for (new slot = 1; slot <= PRACTICE_MAX_TARGETS; slot++)
    {
        if (!PracticePoints[zone][slot][PracticePointSet]) continue;
        slots[total] = slot;
        models[total] = PracticePoints[zone][slot][PracticePointModel];
        noShoot[total++] = PracticePoints[zone][slot][PracticePointNoShoot];
    }
    for (new index = total; index < PRACTICE_MAX_TARGETS; index++)
    {
        slots[index] = 0;
        models[index] = 0;
        noShoot[index] = false;
    }

    // Redistribute the model/behaviour pairs together, independently of order.
    if (PracticeRules[zone][PracticeRandomTypes])
    {
        for (new index = total - 1; index > 0; index--)
        {
            new other = random(index + 1);
            new bool:temporary = noShoot[index];
            noShoot[index] = noShoot[other];
            noShoot[other] = temporary;
            new temporaryModel = models[index];
            models[index] = models[other];
            models[other] = temporaryModel;
        }
    }
    if (PracticeRules[zone][PracticeRandomOrder])
    {
        for (new index = total - 1; index > 0; index--)
        {
            new other = random(index + 1), temporarySlot = slots[index];
            slots[index] = slots[other];
            slots[other] = temporarySlot;
            new bool:temporaryType = noShoot[index];
            noShoot[index] = noShoot[other];
            noShoot[other] = temporaryType;
            new temporaryModel = models[index];
            models[index] = models[other];
            models[other] = temporaryModel;
        }
    }
    return total;
}
