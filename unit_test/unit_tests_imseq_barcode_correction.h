// ============================================================================
// IMSEQ - An immunogenetic sequence analysis tool
// (C) Charite, Universitaetsmedizin Berlin
// Author: Leon Kuchenbecker
// ============================================================================
// 
// This program is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// ============================================================================

// ============================================================================
// FILE DESCRIPTION
// ============================================================================
// Unit tests for barcode_correction.h
// ============================================================================

#include "../src/barcode_correction.h"

SEQAN_DEFINE_TEST(unit_tests_imseq_barcode_correction_splitBarcodeSeq)
{

    {
        unsigned barcodeLength = 10;
        String<Dna5Q> s = "GATCGGTAACGATCGAATGC";
        String<Dna5Q> bc = "AAAA";

        SEQAN_ASSERT(splitBarcodeSeq(s, bc, barcodeLength));
        SEQAN_ASSERT_EQ(bc, "GATCGGTAAC");
        SEQAN_ASSERT_EQ(s, "GATCGAATGC");

        barcodeLength = 21;

        SEQAN_ASSERT(!splitBarcodeSeq(s, bc, barcodeLength));
        SEQAN_ASSERT_EQ(bc, "");
    }
}

SEQAN_DEFINE_TEST(unit_tests_imseq_barcode_correction_splitBarcodeSeq__FastqRecord)
{

    {
        unsigned barcodeLength = 10;

        bool barcodeVDJRead = false;
        FastqRecord<SingleEnd> seRec;
        seRec.seq = "GATCGGTAACGATCGAATGC";
        

        SEQAN_ASSERT(splitBarcodeSeq(seRec, barcodeVDJRead, barcodeLength));
        SEQAN_ASSERT_EQ(seRec.bcSeq, "GATCGGTAAC");
        SEQAN_ASSERT_EQ(seRec.seq, "GATCGAATGC");

        FastqRecord<PairedEnd> peRec;
        peRec.fwSeq = "ACGATACCCTGCATCGGCATGC";
        peRec.revSeq = "TTGGACTATTAGGTAAGTTCGCGAT";
        FastqRecord<PairedEnd> peRecCopy = peRec;

        SEQAN_ASSERT(splitBarcodeSeq(peRec, barcodeVDJRead, barcodeLength));
        SEQAN_ASSERT_EQ(peRec.bcSeq, "ACGATACCCT");
        SEQAN_ASSERT_EQ(peRec.fwSeq, "GCATCGGCATGC");
        SEQAN_ASSERT_EQ(peRec.revSeq, "TTGGACTATTAGGTAAGTTCGCGAT");

        peRec = peRecCopy;
        barcodeVDJRead = true;
        SEQAN_ASSERT(splitBarcodeSeq(peRec, barcodeVDJRead, barcodeLength));
        SEQAN_ASSERT_EQ(peRec.bcSeq, "TTGGACTATT");
        SEQAN_ASSERT_EQ(peRec.fwSeq, "ACGATACCCTGCATCGGCATGC");
        SEQAN_ASSERT_EQ(peRec.revSeq, "AGGTAAGTTCGCGAT");
    }
}

