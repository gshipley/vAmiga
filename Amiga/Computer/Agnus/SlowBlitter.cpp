// -----------------------------------------------------------------------------
// This file is part of vAmiga
//
// Copyright (C) Dirk W. Hoffmann. www.dirkwhoffmann.de
// Licensed under the GNU General Public License v3
//
// See https://www.gnu.org for license information
// -----------------------------------------------------------------------------

#include "Amiga.h"

//
// Micro-instructions
//

/* To keep the implementation flexible, the blitter is emulated as a
 * micro-programmable device. When a blit starts, a micro-program is set up
 * that will decide on the action that are performed in each Blitter cycle.
 *
 * A micro-program consists of the following micro-instructions:
 *
 *     BLTIDLE : Does nothing.
 *     WRITE_D : Writes back D hold.
 *     FETCH_A : Loads register A new.
 *     FETCH_B : Loads register B new.
 *     FETCH_C : Loads register C hold.
 *      HOLD_A : Loads register A hold.
 *      HOLD_B : Loads register B hold.
 *      HOLD_D : Loads register D hold.
 *     BLTDONE : Marks the last instruction.
 *      REPEAT : Continues with the next word.
 *
 * Additional bit masks:
 *
 *         BUS : Indicates that the Blitter needs bus access to proceed.
 */

const uint16_t BUS       = 0b1'0000'0000'0000;

const uint16_t BLTIDLE   = 0b0'0000'0000'0000;
const uint16_t WRITE_D   = 0b0'0000'0100'0000 | BUS;
const uint16_t FETCH_A   = 0b0'0000'0000'0001 | BUS;
const uint16_t FETCH_B   = 0b0'0000'0000'0010 | BUS;
const uint16_t FETCH_C   = 0b0'0000'0000'0100 | BUS;
const uint16_t HOLD_A    = 0b0'0000'0000'1000;
const uint16_t HOLD_B    = 0b0'0000'0001'0000;
const uint16_t HOLD_D    = 0b0'0000'0010'0000;
const uint16_t BLTDONE   = 0b0'0000'1000'0000;
const uint16_t REPEAT    = 0b0'0001'0000'0000;


void
Blitter::startSlowBlitter()
{
    if (bltconLINE()) {

        doFastLineBlit();

        // Schedule the termination event (CLEAN THIS UP)
        agnus->scheduleRel<BLT_SLOT>(0, BLT_FAST_END);

    } else {

        // Setup ascending / descending dependent parameters
        if (bltconDESC()) {
            incr = -2;
            ash  = 16 - bltconASH();
            bsh  = 16 - bltconBSH();
            amod = -bltamod;
            bmod = -bltbmod;
            cmod = -bltcmod;
            dmod = -bltdmod;
        } else {
            incr = 2;
            ash  = bltconASH();
            bsh  = bltconBSH();
            amod = bltamod;
            bmod = bltbmod;
            cmod = bltcmod;
            dmod = bltdmod;
        }

        // Set width and height counters
        resetXCounter();
        resetYCounter();

        // Reset registers
        aold = 0;
        bold = 0;

        // Reset the fill carry bit
        fillCarry = !!bltconFCI();

        // Load the micro-code for this blit
        loadMicrocode();

        // Start the blit
        agnus->scheduleRel<BLT_SLOT>(DMA_CYCLES(1), BLT_EXECUTE);
    }
}

void
Blitter::executeSlowBlitter()
{
    // Only proceed if Blitter DMA is enabled
    if (!agnus->bltDMA()) {
        agnus->cancel<BLT_SLOT>();
        return;
    }

    // Execute next Blitter micro-instruction
    uint16_t instr = microInstr[bltpc] & 0xFFF;
    debug(BLT_DEBUG, "Executing micro instruction %d (%X)\n", bltpc, instr);
    bltpc++;

    if (instr & WRITE_D) {

        /* D is not written in the first iteration, because the pipepline needs
         * to ramp up.
         */
        if (iteration == 0) {

            debug(BLT_DEBUG, "WRITE_D (skipped)\n");

        } else {

            mem->poke16(bltdpt, dhold);
            debug(BLT_DEBUG, "D: poke(%X), %X\n", bltdpt, dhold);
            check1 = fnv_1a_it32(check1, dhold);
            check2 = fnv_1a_it32(check2, bltdpt);

            INC_OCS_PTR(bltdpt, incr);
            if (xCounter == bltsizeW()) INC_OCS_PTR(bltdpt, dmod);
        }
    }

    if (instr & FETCH_A) {

        debug(BLT_DEBUG, "FETCH_A\n");

        // pokeBLTADAT(amiga->mem.peek16(bltapt));
        anew = mem->peek16(bltapt);
        debug(BLT_DEBUG, "    A = peek(%X) = %X\n", bltapt, anew);
        debug(BLT_DEBUG, "    After fetch: A = %X\n", anew);
        INC_OCS_PTR(bltapt, incr);
        if (xCounter == 1) INC_OCS_PTR(bltapt, amod);
    }

    if (instr & FETCH_B) {

        debug(BLT_DEBUG, "FETCH_B\n");

        bnew = mem->peek16(bltbpt);
        debug(BLT_DEBUG, "    B = peek(%X) = %X\n", bltbpt, bnew);
        debug(BLT_DEBUG, "    After fetch: B = %X\n", bnew);
        INC_OCS_PTR(bltbpt, incr);
        if (xCounter == 1) INC_OCS_PTR(bltbpt, bmod);
    }

    if (instr & FETCH_C) {

        debug(BLT_DEBUG, "FETCH_C\n");

        chold = mem->peek16(bltcpt);
        debug(BLT_DEBUG, "    C = peek(%X) = %X\n", bltcpt, chold);
        debug(BLT_DEBUG, "    After fetch: C = %X\n", chold);
        INC_OCS_PTR(bltcpt, incr);
        if (xCounter == 1) INC_OCS_PTR(bltcpt, cmod);
    }

    if (instr & HOLD_A) {

        debug(BLT_DEBUG, "HOLD_A\n");

        debug(BLT_DEBUG, "    After masking with %x (%x,%x) %x\n", mask, bltafwm, bltalwm, anew & mask);

        // Run the barrel shifters on data path A
        debug(BLT_DEBUG, "    ash = %d mask = %X\n", bltconASH(), mask);
        if (bltconDESC()) {
            ahold = HI_W_LO_W(anew & mask, aold) >> ash;
        } else {
            ahold = HI_W_LO_W(aold, anew & mask) >> ash;
        }
        aold = anew & mask;
        debug(BLT_DEBUG, "    After shifting A (%d) A = %x\n", ash, ahold);
    }

    if (instr & HOLD_B) {

        debug(BLT_DEBUG, "HOLD_B\n");

        // Run the barrel shifters on data path B
        debug(BLT_DEBUG, "    bsh = %d\n", bltconBSH());
        if (bltconDESC()) {
            bhold = HI_W_LO_W(bnew, bold) >> bsh;
        } else {
            bhold = HI_W_LO_W(bold, bnew) >> bsh;
        }
        bold = bnew;
        debug(BLT_DEBUG, "    After shifting B (%d) B = %x\n", bsh, bhold);
    }

    if (instr & HOLD_D) {

        debug(BLT_DEBUG, "HOLD_D\n");

        // Run the minterm logic circuit
        debug(BLT_DEBUG, "    Minterms: ahold = %X bhold = %X chold = %X bltcon0 = %X (hex)\n", ahold, bhold, chold, bltcon0);
        dhold = doMintermLogicQuick(ahold, bhold, chold, bltcon0 & 0xFF);
        assert(dhold == doMintermLogic(ahold, bhold, chold, bltcon0 & 0xFF));

        // Run the fill logic circuit
        if (bltconFE()) doFill(dhold, fillCarry);

        // Update the zero flag
        if (dhold) bzero = false;
    }

    if (instr & REPEAT) {

        debug(BLT_DEBUG, "REPEAT\n");

        iteration++;

        // Check if there are remaining words to process
        if (yCounter > 1 || xCounter > 1) {

            // Jump back to the first micro-instruction
            bltpc = 0;


            // Decrease word counters
            if (xCounter > 1) {

                decXCounter();

            } else if (yCounter > 1) {

                resetXCounter();
                decYCounter();
            }

        } else {

            // The remaining code flushes the pipeline.
            // The Blitter busy flag gets cleared at this point.
            bbusy = false;
        }
    }

    if (instr & BLTDONE) {

        debug(BLT_DEBUG, "BLTDONE\n");

        // Clear the Blitter busy flag (if still set)
        bbusy = false;

        // Trigger the Blitter interrupt
        agnus->scheduleRel<IRQ_BLIT_SLOT>(0, IRQ_SET);

        // Terminate the Blitter
        agnus->cancel<BLT_SLOT>();

           plaindebug(BLIT_CHECKSUM, "BLITTER check1: %x check2: %x\n", check1, check2);

    } else {

        // Continue running the Blitter
        // agnus->rescheduleRel<BLT_SLOT>(DMA_CYCLES(1));
    }
}

void
Blitter::setXCounter(uint16_t value)
{
    xCounter = value;

    // Set the mask for this iteration
    mask = 0xFFFF;

    // Apply the "first word mask" in the first iteration
    if (xCounter == bltsizeW()) mask &= bltafwm;

    // Apply the "last word mask" in the last iteration
    if (xCounter == 1) mask &= bltalwm;
}

void
Blitter::setYCounter(uint16_t value)
{
    yCounter = value;
    
    // Reset the fill carry bit
    fillCarry = !!bltconFCI();

    // Apply the "first word mask" in the first iteration
    // mask = bltafwm;
}

void
Blitter::loadMicrocode()
{
    bltpc = 0;
    iteration = 0;

    /* The following code is inspired by Table 6.2 of the HRM:
     *
     *           Active
     * BLTCON0  Channels            Cycle sequence
     *    F     A B C D    A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2
     *    E     A B C      A0 B0 C0 A1 B1 C1 A2 B2 C2
     *    D     A B   D    A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2
     *    C     A B        A0 B0 -- A1 B1 -- A2 B2
     *    B     A   C D    A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2
     *    A     A   C      A0 C0 A1 C1 A2 C2
     *    9     A     D    A0 -- A1 D0 A2 D1 -- D2
     *    8     A          A0 -- A1 -- A2
     *    7       B C D    B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2
     *    6       B C      B0 C0 -- B1 C1 -- B2 C2
     *    5       B   D    B0 -- -- B1 D0 -- B2 D1 -- D2
     *    4       B        B0 -- -- B1 -- -- B2
     *    3         C D    C0 -- -- C1 D0 -- C2 D1 -- D2
     *    2         C      C0 -- C1 -- C2
     *    1           D    D0 -- D1 -- D2
     *    0                -- -- -- --
     */

    uint8_t useA = !!bltconUSEA();
    uint8_t useB = !!bltconUSEB();
    uint8_t useC = !!bltconUSEC();
    uint8_t useD = !!bltconUSED();

    int use = (useA << 3) | (useB << 2) | (useC << 1) | useD;
    switch (use) {

        case 0xF: { // A0 B0 C0 -- A1 B1 C1 D0 A2 B2 C2 D1 D2

            uint16_t prog[] = {

                FETCH_A,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                WRITE_D | HOLD_D | REPEAT,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xE: { // A0 B0 C0 A1 B1 C1 A2 B2 C2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B | REPEAT,

                HOLD_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xD: { // A0 B0 -- A1 B1 D0 A2 B2 D1 -- D2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                FETCH_B | HOLD_A,
                WRITE_D | HOLD_B | REPEAT,

                HOLD_D,
                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xC: { // A0 B0 -- A1 B1 -- A2 B2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                FETCH_B | HOLD_A,
                HOLD_B  | REPEAT,

                HOLD_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xB: { // A0 C0 -- A1 C1 D0 A2 C2 D1 -- D2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                FETCH_C | HOLD_A | HOLD_B,
                WRITE_D | REPEAT,

                HOLD_D,
                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0xA: { // A0 C0 A1 C1 A2 C2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                FETCH_C | HOLD_A | HOLD_B | REPEAT,

                HOLD_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x9: { // A0 -- A1 D0 A2 D1 -- D2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                WRITE_D | HOLD_A | HOLD_B | REPEAT,

                HOLD_D,
                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x8: { // A0 -- A1 -- A2

            uint16_t prog[] = {

                FETCH_A | HOLD_D,
                HOLD_A | HOLD_B | REPEAT,

                HOLD_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x7: { // B0 C0 -- -- B1 C1 D0 -- B2 C2 D1 -- D2

            uint16_t prog[] = {

                FETCH_B | HOLD_A,
                FETCH_C | HOLD_B,
                WRITE_D | HOLD_D,
                REPEAT,

                WRITE_D | BLTDONE


            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x6: { // B0 C0 -- B1 C1 -- B2 C2

            uint16_t prog[] = {

                FETCH_B | HOLD_D,
                FETCH_C | HOLD_A | HOLD_B | REPEAT,

                HOLD_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x5: { // B0 -- -- B1 D0 -- B2 D1 -- D2

            uint16_t prog[] = {

                FETCH_B,
                WRITE_D | HOLD_A | HOLD_B,
                HOLD_D | REPEAT,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x4: { // B0 -- -- B1 -- -- B2

            uint16_t prog[] = {

                FETCH_B,
                HOLD_A | HOLD_B,
                HOLD_D | REPEAT,

                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x3: { // C0 -- -- C1 D0 -- C2 D1 -- D2

            uint16_t prog[] = {

                FETCH_C | HOLD_A | HOLD_B,
                WRITE_D,
                HOLD_D | REPEAT,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x2: { // C0 -- C1 -- C2

            uint16_t prog[] = {

                FETCH_C | HOLD_A | HOLD_B,
                HOLD_D | REPEAT,

                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x1: { // D0 -- D1 -- D2

            uint16_t prog[] = {

                WRITE_D | HOLD_A | HOLD_B,
                HOLD_D | REPEAT,

                WRITE_D | BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        case 0x0: { // -- -- -- --

            uint16_t prog[] = {

                BLTIDLE,
                REPEAT,

                BLTDONE
            };
            memcpy(microInstr, prog, sizeof(prog));
            break;
        }

        default:

            _dump();
            panic("Unimplemented Blitter configuration\n");
            assert(false);
    }

    debug(BLT_DEBUG, "Micro-code loaded (%X)\n", use);
}

void
Blitter::doBarrelShifterA()
{
    uint16_t masked = anew;

    if (isFirstWord()) masked &= bltafwm;
    if (isLastWord())  masked &= bltalwm;

    debug(2, "first = %d last = %d masked = %X\n", isFirstWord(), isLastWord(), masked);

    if(bltconDESC()){
        ahold = (aold >> (16 - bltconASH())) | (masked << bltconASH());
    }else{
        ahold = (aold << (16 - bltconASH())) | (masked >> bltconASH());
    }

    /*
     if (bltDESC()) {
     uint32_t barrelA = HI_W_LO_W(anew & mask, aold);
     ahold = (barrelA >> (16 - bltASH())) & 0xFFFF;
     } else {
     uint32_t barrelA = HI_W_LO_W(aold, anew & mask);
     ahold = (barrelA >> bltASH()) & 0xFFFF;
     }
     */
}

void
Blitter::doBarrelShifterB()
{
    if(bltconDESC()) {
        bhold = (bold >> (16 - bltconBSH())) | (bnew << bltconBSH());
    } else {
        bhold = (bold << (16 - bltconBSH())) | (bnew >> bltconBSH());
    }

    /*
     if (bltDESC()) {
     uint32_t barrelB = HI_W_LO_W(bnew, bold);
     bhold = (barrelB >> (16 - bltBSH())) & 0xFFFF;
     } else {
     uint32_t barrelB = HI_W_LO_W(bold, bnew);
     bhold = (barrelB >> bltBSH()) & 0xFFFF;
     }
     */
}