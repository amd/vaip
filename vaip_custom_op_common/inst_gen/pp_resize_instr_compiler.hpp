/*
 *  Copyright (C) 2023 – 2024 Advanced Micro Devices, Inc. All rights reserved.
 *  Licensed under the MIT License.
 */
#pragma once

#include "aie2_instr_ir_writer.hpp"
#include "aie2_ipu_debug_instr_writer.hpp"
#include "aie2_ipu_instr_writer.hpp"
#include "pp_instr_compiler_impl.hpp"

class ResizeInstrCompiler : public InstructionCompiler<ResizeInstrCompiler> {
public:
  // Ctor
  ResizeInstrCompiler(std::uint32_t* instr_buffer)
      : InstructionCompiler(instr_buffer) {}
  // TODO:: Add const for the rtps buffer.
  int generate(std::uint16_t* rtps);
};

// TODO: create a separate copy of the rtps and must not change the one coming
// from app.
int ResizeInstrCompiler::generate(std::uint16_t* rtps) {

  // Data from RTPs
  int16_t img_width_in = rtps[PP_RTP_IMG_WIDTH_IN];
  int16_t img_height_in = rtps[PP_RTP_IMG_HEIGHT_IN];
  int16_t img_width_out = rtps[PP_RTP_IMG_WIDTH_OUT];
  int16_t img_height_out = rtps[PP_RTP_IMG_HEIGHT_OUT];
  int16_t channels_ = rtps[PP_RESIZE_DOWN_RTP_CHANNELS];

  uint32_t tile_width_in = img_width_in;
  uint32_t tile_height_in = 2;
  uint32_t tile_width_out = img_width_out;
  uint32_t tile_height_out = 1;

  uint32_t TILE_WINDOW_SIZE_IN1 = (channels_ * tile_width_in);
  uint32_t TILE_WINDOW_SIZE_OUT =
      (channels_ * tile_width_out * tile_height_out);

  uint32_t TILE_WINDOW_SIZE_IN2 = TILE_WINDOW_SIZE_IN1;
  uint32_t TILE_WINDOW_SIZE_IN = TILE_WINDOW_SIZE_IN1 + TILE_WINDOW_SIZE_IN2;

  int RTP_ADDR = 0x2000;

  // DMA components
  std::vector<uint32_t> dma_components;
  dma_components.reserve(1000);

  AIE2::TileMetaData shape_in = {1, tile_width_in, (uint32_t)channels_};
  // Fill this
  std::vector<AIE2::TileMetaData> tiles_in;

  tiles_in.emplace_back(shape_in);
  tiles_in.emplace_back(shape_in);

  AIE2::TileMetaData shape_out = {tile_height_out, tile_width_out,
                                  (uint32_t)channels_};
  // Fill this
  std::vector<AIE2::TileMetaData> tiles_out;

  tiles_out.emplace_back(shape_out);

  // Initialize compiler
  init(tiles_in, tiles_out);
  ////
  // AIE and Mem Tile config
  ////
  generateInitMemDma(dma_components);
  generateInitAieDma(dma_components);
  generateAieDma(dma_components);
  generateMemDma1(dma_components);

  ////
  // shim tile config
  ////

  // Set ShimDMA for input and output
  float y_scale = (float)img_height_in / (float)img_height_out;
  int num_tiles = img_height_out / tile_height_out;
  // TODO:: check if the param_buffer is really necessary
  rtps[PP_RTP_NUM_TILES_PER_CORE] = (num_tiles + 3) / 4;
  rtps[PP_RTP_NEXT_OPCODE_POS] = 0;
  for (int i = 0; i < 4; i++) {
    AIE2::Location at_location(0, 2 + i);
    auto word = m_aie_dma->getWord((m_rtp_addr + 2 * PP_RTP_NUM_TILES_PER_CORE),
                                   rtps[PP_RTP_NUM_TILES_PER_CORE]);
    word->write(at_location);
#ifdef USE_VISITOR
    dma_components.push_back(word->getUniqueId());
#else
    m_instr_counter +=
        m_instr_writer->write(word, m_instr_buffer + m_instr_counter);
#endif
  }
  constexpr int maxBDsPerBatch = 2;
  constexpr int numBDsPerBatch = 2;
  constexpr int numTilesPerBD = 2;

  uint16_t in_out_rows[2];
  uint32_t* io_row_ptr = (uint32_t*)(in_out_rows);

  AIE2::DmaIteration nt_iteration;
  AIE2::Location nt_location(0, 0);
  uint8_t nt_ncols = 1, ddr_type;

  for (int tile = 0; tile < num_tiles;) {
    // std::cout << "-- Tile: " << tile << "\n";
    int batch = tile / 4;
    tile = tile - (4 + tile - num_tiles) * ((num_tiles - tile) < 4);
    int StartBd = 5 * (batch % 3);

    AIE2::DmaTensor4D nt_out_tensor = {{{1, 0}, {1, 0}, {1, 0}, {}}};

    nt_iteration.m_current = 0;
    nt_iteration.m_step = (m_tile_window_size_out_total >> 2);
    nt_iteration.m_wrap = (numBDsPerBatch * numTilesPerBD);

    ddr_type = 1;

    // output BD
    // -------------------------------------------------------------------------
    uint32_t start_out_bd = StartBd + 2 * numBDsPerBatch;
    uint32_t repeat_out_bd = numBDsPerBatch * numTilesPerBD;

    auto outBD = m_noc_dma->getBD(start_out_bd);
    outBD->updateAddr((tile * m_tile_window_size_out_total), 0)
        ->updateLength(m_tile_window_size_out_total);
    outBD->updateTensorDims(nt_out_tensor)->updateIterationDims(nt_iteration);
    outBD->write(nt_location, nt_ncols, ddr_type);

    // task queue
    auto outTaskQ = m_noc_dma->getS2MMQueue(0);
    outTaskQ->setStartBd(start_out_bd)->setRepeatCount(repeat_out_bd);
    outTaskQ->enableTokenIssue()->write(nt_location);

#ifdef USE_VISITOR
    // Push to vector
    dma_components.push_back(outBD->getUniqueId());
    dma_components.push_back(outTaskQ->getUniqueId());
#else
    m_instr_counter +=
        m_instr_writer->write(outBD, m_instr_buffer + m_instr_counter);
    m_instr_counter +=
        m_instr_writer->write(outTaskQ, m_instr_buffer + m_instr_counter);
#endif

    for (int j = 0; j < numBDsPerBatch; tile++, j++) {
      // Metadata compute done here
      float idx_y = ((tile + j + 0.5f) * y_scale) - 0.5f;
      idx_y = PP_MIN(PP_MAX(idx_y, 0.0f), (float)(img_height_in - 2));
      int offset_y = (int)idx_y;

      float idx_y1 = ((tile + 1 + j + 0.5f) * y_scale) - 0.5f;
      idx_y1 = PP_MIN(PP_MAX(idx_y1, 0.0f), (float)(img_height_in - 2));
      int offset_y1 = (int)idx_y1;

      // std::cout << "   Tile: " << tile + j << " Rows: " << offset_y << ", "
      // << offset_y + 1 << "\n";

      in_out_rows[0] = offset_y;
      in_out_rows[1] = tile + j;

      AIE2::Location at_loc_1(0, 2 + 2 * (j % 2));
      auto word_1 =
          m_aie_dma->getWord((RTP_ADDR + 2 * (PP_RESIZE_DOWN_RTP_IN_ROW_0 +
                                              2 * ((j / 2) + (batch) % 3))),
                             io_row_ptr[0]);
      word_1->write(at_loc_1);

#ifdef USE_VISITOR
      dma_components.push_back(word_1->getUniqueId());
#else
      m_instr_counter +=
          m_instr_writer->write(word_1, m_instr_buffer + m_instr_counter);
#endif

      in_out_rows[0] = offset_y1;
      in_out_rows[1] = tile + 1 + j;

      AIE2::Location at_loc_2(0, 2 + 2 * (j % 2) + 1);
      auto word_2 =
          m_aie_dma->getWord((RTP_ADDR + 2 * (PP_RESIZE_DOWN_RTP_IN_ROW_0 +
                                              2 * ((j / 2) + (batch) % 3))),
                             io_row_ptr[0]);
      word_2->write(at_loc_2);

#ifdef USE_VISITOR
      dma_components.push_back(word_2->getUniqueId());
#else
      m_instr_counter +=
          m_instr_writer->write(word_2, m_instr_buffer + m_instr_counter);
#endif

      idx_y = ((tile + 0.5f) * y_scale) - 0.5f;
      idx_y = PP_MIN(PP_MAX(idx_y, 0.0f), (float)(img_height_in - 2));
      offset_y = (int)idx_y;

      idx_y1 = ((tile + numBDsPerBatch + 0.5f) * y_scale) - 0.5f;
      idx_y1 = PP_MIN(PP_MAX(idx_y1, 0.0f), (float)(img_height_in - 2));
      offset_y1 = (int)idx_y1;

      uint32_t addr_offset = ((offset_y1 * img_width_in * channels_) -
                              (offset_y * img_width_in * channels_));

      uint32_t addr_low, addr_high;
      uint8_t use_next_bd, bd_id, next_bd, valid_bd;

      // Input-0 (y)
      // -------------------------------------------------------------------------
      nt_iteration.m_current = 0;
      nt_iteration.m_step = (addr_offset >> 2);
      nt_iteration.m_wrap = (addr_offset < 4) ? 1 : (numTilesPerBD);

      use_next_bd = (j < (numBDsPerBatch - 1)) ? 1 : 0;
      bd_id = StartBd + 0 + j;
      next_bd = (j < (numBDsPerBatch - 1)) ? (StartBd + 0 + j + 1) : 0;
      valid_bd = 1;
      ddr_type = 0;

      addr_low = (offset_y * img_width_in * channels_);
      addr_high = 0;

      auto y0BD = m_noc_dma->getBD(bd_id);
      y0BD->updateAddr(addr_low, addr_high)
          ->updateLength(m_tile_window_size_in[0]);
      y0BD->updateTensorDims(nt_out_tensor)->updateIterationDims(nt_iteration);
      if (use_next_bd)
        y0BD->enableNextBD()->updateNextBD(next_bd);
      y0BD->write(nt_location, nt_ncols, ddr_type);

#ifdef USE_VISITOR
      dma_components.push_back(y0BD->getUniqueId());
#else
      m_instr_counter +=
          m_instr_writer->write(y0BD, m_instr_buffer + m_instr_counter);
#endif

      // Input-1 (uv)
      // -------------------------------------------------------------------------

      nt_iteration.m_step = (addr_offset >> 2);
      nt_iteration.m_wrap = (addr_offset < 4) ? 1 : (numTilesPerBD);

      use_next_bd = (j < (numBDsPerBatch - 1)) ? 1 : 0;
      bd_id = StartBd + maxBDsPerBatch + j;
      next_bd =
          (j < (numBDsPerBatch - 1)) ? (StartBd + maxBDsPerBatch + j + 1) : 0;
      valid_bd = 1;
      ddr_type = 0;

      addr_low =
          ((offset_y * img_width_in * channels_) + (img_width_in * channels_));
      addr_high = 0;

      auto y1BD = m_noc_dma->getBD(bd_id);
      y1BD->updateAddr(addr_low, addr_high)
          ->updateLength(m_tile_window_size_in[1]);
      y1BD->updateTensorDims(nt_out_tensor)->updateIterationDims(nt_iteration);
      if (use_next_bd)
        y1BD->enableNextBD()->updateNextBD(next_bd);
      y1BD->write(nt_location, nt_ncols, ddr_type);

#ifdef USE_VISITOR
      dma_components.push_back(y1BD->getUniqueId());
#else
      m_instr_counter +=
          m_instr_writer->write(y1BD, m_instr_buffer + m_instr_counter);
#endif
    }

    // Start Q word
    auto in1TaskQ = m_noc_dma->getMM2SQueue(0);
    in1TaskQ->setStartBd(StartBd)
        ->setRepeatCount(numTilesPerBD)
        ->write(nt_location);

#ifdef USE_VISITOR
    dma_components.push_back(in1TaskQ->getUniqueId());
#else
    m_instr_counter +=
        m_instr_writer->write(in1TaskQ, m_instr_buffer + m_instr_counter);
#endif

    // Start Q word
    auto in2TaskQ = m_noc_dma->getMM2SQueue(1);
    in2TaskQ->setStartBd(StartBd + maxBDsPerBatch)
        ->setRepeatCount(numTilesPerBD)
        ->write(nt_location);

#ifdef USE_VISITOR
    dma_components.push_back(in2TaskQ->getUniqueId());
#else
    m_instr_counter +=
        m_instr_writer->write(in2TaskQ, m_instr_buffer + m_instr_counter);
#endif

    // Update tile counter
    tile += numBDsPerBatch;
    if (batch > 1) {
      // Sync
      auto sync_word = m_noc_dma->getSyncWord(AIE2::DmaDirection::S2MM, 0, 1,
                                              1); // (direction, ch, ncol, nrow)
      sync_word->write(nt_location);
#ifdef USE_VISITOR
      dma_components.push_back(sync_word->getUniqueId());
#else
      m_instr_counter +=
          m_instr_writer->write(sync_word, m_instr_buffer + m_instr_counter);
#endif
    }
  }
  auto sync_word_1 = m_noc_dma->getSyncWord(AIE2::DmaDirection::S2MM, 0, 1,
                                            1); // (direction, ch, ncol, nrow)
  auto sync_word_2 = m_noc_dma->getSyncWord(AIE2::DmaDirection::S2MM, 0, 1,
                                            1); // (direction, ch, ncol, nrow)
  sync_word_1->write(nt_location);
  sync_word_1->write(nt_location);

#ifdef USE_VISITOR
  dma_components.push_back(sync_word_1->getUniqueId());
  dma_components.push_back(sync_word_2->getUniqueId());
#else
  m_instr_counter +=
      m_instr_writer->write(sync_word_1, m_instr_buffer + m_instr_counter);
  m_instr_counter +=
      m_instr_writer->write(sync_word_2, m_instr_buffer + m_instr_counter);
#endif

#ifdef USE_VISITOR
  // Visitors
  auto instrWriter = std::make_unique<IPUInstructionWriter>();
  // auto irWriter = std::make_unique<InstructionIRWriter>();
  auto dbgWriter = std::make_unique<IPUDebugInstrWriter>();
  // std::cout << "-- DMA Components: " << dma_components.size() << "\n";
  m_instr_counter = 0;
  for (auto uid : dma_components) {
    auto component = getDmaElement(uid);
    auto ncount =
        component->accept(instrWriter.get(), m_instr_buffer + m_instr_counter);
    auto ncount1 =
        component->accept(dbgWriter.get(), m_instr_buffer + m_instr_counter);
    // auto rval1 = component->accept(irWriter.get(), m_instr_buffer +
    // m_instr_counter);
    m_instr_counter += ncount;
  }
  // Write to console
  // dbgWriter->writeToConsole();
  dbgWriter->writeToFile("output_dbg.txt");
#endif
  // Return number of instructions generated
  return m_instr_counter;
}
