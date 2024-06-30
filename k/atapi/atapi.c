#include "../io.h"
#include "../memory.h"
#include "atapi.h"
#include <k/blockdev.h>
#include <stdio.h>
#include <string.h>

#define ATAPI_SIGNATURE_LEN 4

struct atapi_drive {
  u16 reg_type;
  u8 ata_port;
};

struct atapi_drive atapi_drive;
struct SCSI_packet pkt;

#define LEN_ARRAY(arr) (sizeof(arr) / sizeof(arr[0]))
#define IS_ATAPI_FOUND (atapi_drive.reg_type != 0)

static void busy_wait(u16 reg) {
  // Wait for BSY to be cleared
  while (inb(ATA_REG_STATUS(reg)) & BSY)
    ;
}

static void drq_wait(u16 reg) {
  // Wait for DRQ to be set
  while (!(inb(ATA_REG_STATUS(reg)) & DRQ))
    ;
}

static void wait_packet_request(u16 reg) {
  // printf("Waiting for packet request\n");
  busy_wait(reg);
  // printf("Busy wait done\n");
  drq_wait(reg);
  // printf("DRQ wait done\n");
}

static void wait_command_complete(u16 reg) {
  // Wait for the command to complete
  while (inb(ATA_REG_SECTOR_COUNT(reg)) != PACKET_COMMAND_COMPLETE)
    ;
}

static void select_drive(u16 reg, u8 ata_port) {
  // Select the drive by setting the head register
  outb(ATA_REG_DRIVE(reg), ata_port);

  // Wait until not busy anymore (drive is ready)
  busy_wait(reg);
}

static int is_atapi_drive(u16 reg, u8 ata_port) {
  // Select the drive
  select_drive(reg, ata_port);

  // Signature bytes container
  u8 sig[ATAPI_SIGNATURE_LEN];

  // Fetch ATAPI signature bytes
  sig[0] = inb(ATA_REG_SECTOR_COUNT(reg));
  sig[1] = inb(ATA_REG_LBA_LO(reg));
  sig[2] = inb(ATA_REG_LBA_MI(reg));
  sig[3] = inb(ATA_REG_LBA_HI(reg));

  // Compare the signature with the ATAPI signature
  u8 atapi_sig[ATAPI_SIGNATURE_LEN] = {ATAPI_SIG_SC, ATAPI_SIG_LBA_LO,
                                       ATAPI_SIG_LBA_MI, ATAPI_SIG_LBA_HI};

  // Compare the signature with the ATAPI signature & return the result
  return memcmp(sig, atapi_sig, sizeof(atapi_sig)) == 0;
}

int discover_atapi_drive(void) {
  const u16 reg_types[] = {PRIMARY_REG, SECONDARY_REG};
  const u8 ata_ports[] = {ATA_PORT_MASTER, ATA_PORT_SLAVE};

  for (size_t i = 0; i < LEN_ARRAY(reg_types); ++i) {
    u16 reg = reg_types[i];
    u16 dcr = (reg == PRIMARY_REG) ? PRIMARY_DCR : SECONDARY_DCR;

    // Prepare the drive for discovery
    outb(dcr, SRST);              // Software reset
    outb(dcr, INTERRUPT_DISABLE); // Disable interrupts

    for (size_t j = 0; j < LEN_ARRAY(ata_ports); ++j) {
      u8 ata_port = ata_ports[j];

      // Check if it's an ATAPI drive
      if (is_atapi_drive(reg, ata_port)) {
        // Clear the ATAPI drive structure
        memset(&atapi_drive, 0, sizeof(atapi_drive));

        // Save the found drive
        atapi_drive.reg_type = reg;      // Found reg (Primary/Secondary)
        atapi_drive.ata_port = ata_port; // Found ATA port (Master/Slave)
        printf("ATAPI found (reg=0x%03x, ata_port=0x%03x)\n",
               atapi_drive.reg_type, atapi_drive.ata_port);

        // Exit since the drive has been found
        return 0;
      }
    }
  }

  // No ATAPI drive found
  printf("No ATAPI drive found\n");
  return -1;
}

static void setup_SCSI_packet(struct SCSI_packet *pkt, u8 cmd, size_t lba,
                              u32 nb_blocks) {
  // Clear the packet structure
  memset(pkt, 0, sizeof(struct SCSI_packet));

  // Set the command
  pkt->op_code = cmd;

  // Set the LBA
  pkt->lba_hi = (lba >> 0x18) & 0xFF;
  pkt->lba_mihi = (lba >> 0x10) & 0xFF;
  pkt->lba_milo = (lba >> 0x08) & 0xFF;
  pkt->lba_lo = lba & 0xFF;

  // Set the transfer length
  pkt->transfer_length_hi = (nb_blocks >> 24) & 0xFF;
  pkt->transfer_length_mihi = (nb_blocks >> 16) & 0xFF;
  pkt->transfer_length_milo = (nb_blocks >> 8) & 0xFF;
  pkt->transfer_length_lo = nb_blocks & 0xFF;
}

int send_SCSI_packet(struct SCSI_packet *pkt, u16 reg, u16 size) {
  if (!IS_ATAPI_FOUND) {
    printf("No ATAPI drive found. Cannot send SCSI packet.\n");
    return -1;
  }

  // Wait for the ATAPI drive to become ready
  busy_wait(reg);
  // printf("ATAPI drive ready\n");

  // No overlap/no DMA
  outb(ATA_REG_FEATURES(reg), 0);
  // printf("No overlap/no DMA\n");

  // Wait for the device to request data transmission & clear the sector count
  busy_wait(reg); // not sure if this is needed
  outb(ATA_REG_SECTOR_COUNT(reg), 0);
  // printf("No queuing\n");

  // Set the CDROM block size
  outb(ATA_REG_LBA_MI(reg), size);
  outb(ATA_REG_LBA_HI(reg), size >> 8);
  // printf("CDROM block size set\n");

  // Send the PACKET command
  outb(ATA_REG_COMMAND(reg), PACKET);
  // printf("Sent PACKET command\n");

  // Wait for the device to request a packet
  wait_packet_request(reg);
  // printf("Device requested a packet\n");

  // Send the packet (word by word)
  outsw(ATA_REG_DATA(reg), (u16 *)pkt, PACKET_SZ / 2);
  printf("Packet %d sent\n", pkt->op_code);
  return 0;
}

int send_read_block_command(u16 reg, size_t lba) {
  // Make sure an ATAPI drive has been found before sending the command
  if (!IS_ATAPI_FOUND) {
    printf("No ATAPI drive found. Cannot read block.\n");
    return -1;
  }

  // Select the drive
  select_drive(reg, atapi_drive.ata_port);

  // Setup and send the SCSI packet
  struct SCSI_packet pkt;
  setup_SCSI_packet(&pkt, READ_12, lba, 1);
  // printf("SCSI packet setup\n");

  if (send_SCSI_packet(&pkt, atapi_drive.reg_type, CD_BLOCK_SZ) != 0) {
    printf("Failed to send SCSI packet.\n");
    return -1;
  } else
    printf("SCSI packet successfully sent\n");

  return 0; // Success
}

void *read_block(size_t lba) {
  // Make sure an ATAPI drive has been found before reading the block
  if (!IS_ATAPI_FOUND) {
    printf("No ATAPI drive found. Cannot read block.\n");
    return NULL;
  }

  // Allocate memory for the block
  void *buffer = memory_reserve(CD_BLOCK_SZ);
  printf("Memory reserved for fetching block %d\n", lba);

  // Send the read block command
  if (send_read_block_command(atapi_drive.reg_type, lba) != 0) {
    printf("Failed to send read block command.\n");
    return NULL;
  }

  // Read CDROM block size word by word
  for (size_t i = 0; i < CD_BLOCK_SZ / 2; i++)
    ((u16 *)buffer)[i] = inw(ATA_REG_DATA(atapi_drive.reg_type));

  // Wait for the command to complete
  wait_command_complete(atapi_drive.reg_type);
  printf("Reading of block %d complete\n", lba);

  // Print the block content as hexadecimal values
  printf("Block successfully read\n");
  return buffer; // Success
}
