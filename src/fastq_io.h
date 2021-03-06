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
// HEADER FILE DESCRIPTION
// ============================================================================
// Reading FASTA / FASTQ files
// ============================================================================

#ifndef IMSEQ_FASTQ_IO_H
#define IMSEQ_FASTQ_IO_H

#include <string>
#include <sstream>

#include <seqan/basic.h>
#include <seqan/sequence.h>
#include <seqan/seq_io.h>

#include "sequence_data.h"
#include "barcode_correction.h"
#include "reject.h"
#include "collection_utils.h"
#include "qc_basics.h"
#include "runtime_options.h"


/********************************************************************************
 * STRUCTS AND CLASSES
 ********************************************************************************/

#include "fastq_io_types.h"

/********************************************************************************
 * FUNCTIONS
 ********************************************************************************/

/**
 * Separate sequence and barcode which is a prefix of the sequence
 * @param       seq     The original sequence to be modified
 * @param     bcSeq     The object to store the barcode sequence in
 * @param barcodeLength Length of the barcode
 */
template<typename TSequence>
bool splitBarcodeSeq(TSequence & seq, TSequence & bcSeq, unsigned const barcodeLength) {
    typedef typename Infix<TSequence>::Type TInfix;

    if (barcodeLength == 0)
        return true;

    if (length(seq) < barcodeLength) {
        resize(bcSeq, 0);
        return false;
    }

    TInfix bc = infix(seq, 0, barcodeLength);
    bcSeq = bc;

    TInfix seqInfix = infix(seq, barcodeLength, length(seq));
    TSequence _seq = seqInfix;

    seq = _seq;

    return true;
}

inline bool splitBarcodeSeq(FastqRecord<SingleEnd> & rec, bool const barcodeVDJRead, unsigned const barcodeLength) {
    ignoreUnusedVariableWarning(barcodeVDJRead);
    return splitBarcodeSeq(rec.seq, rec.bcSeq, barcodeLength);
}

inline bool splitBarcodeSeq(FastqRecord<PairedEnd> & rec, bool const barcodeVDJRead, unsigned const barcodeLength) {
    if (barcodeVDJRead)
        return splitBarcodeSeq(rec.revSeq, rec.bcSeq, barcodeLength);
    else
        return splitBarcodeSeq(rec.fwSeq, rec.bcSeq, barcodeLength);
}

/**
 * Computes the size of an input file
 */
inline uint64_t computeFileSize(std::string const & path) {
    SeqFileIn seqFileIn;
    open(seqFileIn, path.c_str());
    uint64_t res = 0;
    for (; !atEnd(seqFileIn.iter); ++seqFileIn.iter)
        ++res;
    return res;
}

/**
 * Returns the longer sequence of a FastqRecord. Simply returns the sequence
 * for a single end record, the longer sequence of the two for a paired end
 * record.
 *
 * @special Single end
 */
inline FastqRecord<SingleEnd>::TSequence & longerSeq(FastqRecord<SingleEnd> & rec)
{
    return rec.seq;
}

/**
 * @special Paired end
 */
inline FastqRecord<PairedEnd>::TSequence & longerSeq(FastqRecord<PairedEnd> & rec)
{
    if (length(rec.fwSeq) > length(rec.revSeq))
        return rec.fwSeq;
    return rec.revSeq;
}

/**
 * Returns the longer sequence of a FastqRecord. Simply returns the sequence
 * for a single end record, the longer sequence of the two for a paired end
 * record.
 *
 * @special Single end
 */
inline FastqRecord<SingleEnd>::TSequence & shorterSeq(FastqRecord<SingleEnd> & rec)
{
    return rec.seq;
}

/**
 * @special Paired end
 */
inline FastqRecord<PairedEnd>::TSequence & shorterSeq(FastqRecord<PairedEnd> & rec)
{
    if (length(rec.fwSeq) < length(rec.revSeq))
        return rec.fwSeq;
    return rec.revSeq;
}

/**
 * Truncate the sequences of a FastqRecord
 *
 * @special Single end
 */
inline void truncate(FastqRecord<SingleEnd> & rec, size_t len)
{
    if (length(rec.seq) > len)
        resize(rec.seq, len);
}

/**
 * @special Paired end
 */
inline void truncate(FastqRecord<PairedEnd> & rec, size_t len)
{
    if (length(rec.fwSeq) > len)
        resize(rec.fwSeq, len);
    if (length(rec.revSeq) > len)
        resize(rec.revSeq, len);
}

/**
 * By default, the V-read is unmodified and the V(D)J read is reverse
 * complemented. If -r was specified, the opposite is performed.
 */
inline void syncOrientation(FastqRecord<PairedEnd> & fqRecord, CdrOptions const & options)
{
    if (options.reverse)
        reverseComplement(fqRecord.fwSeq);
    else
        reverseComplement(fqRecord.revSeq);
}

/**
 * By default, the V(D)J-read is reverse complemented. If -r is specified, this is omitted.
 */
inline void syncOrientation(FastqRecord<SingleEnd> & fqRecord, CdrOptions const & options)
{
    if (!options.reverse)
        reverseComplement(fqRecord.seq);
}

/**
 * Create a string representation of a FastqRecord
 * @special Paired end implementation
 */
inline std::string toString(FastqRecord<PairedEnd> const & rec) {
    std::stringstream ss;
    ss << "BARCODE\t" << rec.bcSeq << "\tFORWARD\t" << rec.fwSeq
            << "\tREVERSE\t" << rec.revSeq;
    return ss.str();
}

/**
 * Create a string representation of a FastqRecord
 * @special Single end implementation
 */
inline std::string toString(FastqRecord<SingleEnd> const & rec) {
    std::stringstream ss;
    ss << "BARCODE\t" << rec.bcSeq << "\tREAD\t" << rec.seq;
    return ss.str();
}

inline bool readTooShort(FastqRecord<SingleEnd> & rec, size_t const minLength, bool const singleEndFallback = false)
{
    ignoreUnusedVariableWarning(singleEndFallback);
    return length(rec.seq) < minLength; 
}

inline bool readTooShort(FastqRecord<PairedEnd> & rec, size_t const minLength, bool const singleEndFallback = false)
{
    if (length(rec.revSeq) < minLength)
        return true;
    if (length(rec.fwSeq) >= minLength)
        return false;
    if (!singleEndFallback)
        return true;
    rec.fwSeq = "";
    return false;
}

/**
 * Performs the first quality control performed directly after reading a FASTQ
 * record.
 *
 * @param     rec The FASTQ record to check
 * @param options The runtime options that contain the user spec thresholds
 *
 * @return A RejectReason object
 */
template <typename TSequencingSpec>
RejectReason qualityControl(FastqRecord<TSequencingSpec> & rec,
        CdrOptions const & options)
{
    if (contains(rec.bcSeq, 'N'))
        return N_IN_BARCODE;
    if (options.bcQmin > 0 && anyQualityBelow(rec.bcSeq, options.bcQmin))
        return LOW_QUALITY_BARCODE_BASE;
    if (options.qmin > 0 && averageQualityBelow(rec, options.qmin, options.singleEndFallback))
        return AVERAGE_QUAL_FAIL;
    if (readTooShort(rec, options.minReadLength, options.singleEndFallback))
        return READ_TOO_SHORT;
    return NONE;
}

inline uint64_t approxSizeInBytes(FastqRecord<SingleEnd> const & rec) {
    return 2*length(rec.seq) + 2*length(rec.bcSeq) + length(rec.id) + 6;
}

inline uint64_t approxSizeInBytes(FastqRecord<PairedEnd> const & rec) {
    return 2*length(rec.fwSeq) + 2*length(rec.revSeq) + 2*length(rec.bcSeq) + 2*length(rec.id) + 12;
}

/**
 * Read a single FASTQ record from a SeqInputStreams object
 * @special Paired end implementation
 */
inline void readRecord(FastqRecord<PairedEnd> & fastqRecord, SeqInputStreams<PairedEnd> & inStreams) {
    try {
        readRecord(fastqRecord.id, fastqRecord.revSeq, inStreams.revStream);
    } catch (IOError e) {
        throw std::string("An I/O error occurred: ") + std::string(e.what());
    } catch (ParseError e) {
        throw std::string("Could not parse FASTQ file '") + inStreams.revPath + std::string("': ") + std::string(e.what());
    }
    try {
        readRecord(fastqRecord.id, fastqRecord.fwSeq, inStreams.fwStream);
    } catch (IOError e) {
        throw std::string("An I/O error occurred: ") + std::string(e.what());
    } catch (ParseError e) {
        throw std::string("Could not parse FASTQ file '") + inStreams.fwPath + std::string("': ") + std::string(e.what());
    }
}

/**
 * Read a single FASTQ record from a SeqInputStreams object
 * @special Single end implementation
 */
inline void readRecord(FastqRecord<SingleEnd> & fastqRecord, SeqInputStreams<SingleEnd> & inStreams) {
    try {
        readRecord(fastqRecord.id, fastqRecord.seq, inStreams.stream);
    } catch (IOError e) {
        throw std::string("An I/O error occurred: ") + std::string(e.what());
    } catch (ParseError e) {
        throw std::string("Could not parse FASTQ file '") + inStreams.path + std::string("': ") + std::string(e.what());
    }
}

/**
 * Read a single FASTQ record from a SeqInputStreams object and split barcode
 * @special Single end implementation
 */
inline bool readRecord(FastqRecord<SingleEnd> & fastqRecord, SeqInputStreams<SingleEnd> & inStreams, bool const barcodeVDJRead, unsigned const barcodeLength) {
    readRecord(fastqRecord, inStreams);
    return splitBarcodeSeq(fastqRecord, barcodeVDJRead, barcodeLength);
}

/**
 * Read a single FASTQ record from a SeqInputStreams object and split barcode
 * @special Paired end implementation
 */
inline bool readRecord(FastqRecord<PairedEnd> & fastqRecord, SeqInputStreams<PairedEnd> & inStreams, bool const barcodeVDJRead, unsigned const barcodeLength) {
    readRecord(fastqRecord.id, fastqRecord.revSeq, inStreams.revStream);
    readRecord(fastqRecord.id, fastqRecord.fwSeq, inStreams.fwStream);
    return splitBarcodeSeq(fastqRecord, barcodeVDJRead, barcodeLength);
}

#endif
