/**
  ******************************************************************************
  * @file    nema_hal.c
  * @brief   NemaGFX platform interfaces with ThreadX support.
  ******************************************************************************
  */

#include <touchgfx/hal/Config.hpp>
#include <nema_sys_defs.h>
#include <nema_core.h>

#include <assert.h>
#include <string.h>

#include "stm32n6xx_hal.h"
#include "tx_api.h"
#include "app_azure_rtos_config.h"

#include "tsi_malloc.h"

#define RING_SIZE                      1024
#define NEMAGFX_MEM_POOL_SIZE          18432

LOCATION_PRAGMA_NOLOAD("Nemagfx_Memory_Pool_Buffer")
static uint8_t nemagfx_pool_mem[NEMAGFX_MEM_POOL_SIZE] LOCATION_ATTRIBUTE_NOLOAD("Nemagfx_Memory_Pool_Buffer");

static nema_ringbuffer_t ring_buffer_str;
volatile static int last_cl_id = -1;
extern GPU2D_HandleTypeDef hgpu2d;

TX_SEMAPHORE nema_irq_sem;

uint32_t nema_reg_read(uint32_t reg);
void nema_reg_write(uint32_t reg, uint32_t value);

#if (USE_HAL_GPU2D_REGISTER_CALLBACKS == 1)
static void GPU2D_CommandListCpltCallback(GPU2D_HandleTypeDef* hgpu2d, uint32_t CmdListID)
#else
void HAL_GPU2D_CommandListCpltCallback(GPU2D_HandleTypeDef* hgpu2d, uint32_t CmdListID)
#endif
{
    UNUSED(hgpu2d);

    last_cl_id = CmdListID;
    tx_semaphore_put(&nema_irq_sem);
}

void HAL_GPU2D_ErrorCallback(GPU2D_HandleTypeDef* hgpu2d)
{
    uint32_t val;

    UNUSED(hgpu2d);

    val = nema_reg_read(GPU2D_SYS_INTERRUPT);
    nema_reg_write(GPU2D_SYS_INTERRUPT, val);

    if (val & (1UL << 2))
    {
        HAL_ICACHE_Disable();
        nema_ext_hold_deassert_imm(2);
    }

    if (val & (1UL << 3))
    {
        HAL_ICACHE_Enable();
        HAL_ICACHE_Invalidate();
        nema_ext_hold_deassert_imm(3);
    }
}

void platform_disable_cache(void)
{
    nema_ext_hold_assert(2, 1);
}

void platform_invalidate_cache(void)
{
    nema_ext_hold_assert(3, 1);
}

int32_t nema_sys_init(void)
{
    int error_code = 0;

#if (USE_HAL_GPU2D_REGISTER_CALLBACKS == 1)
    HAL_GPU2D_RegisterCommandListCpltCallback(&hgpu2d, GPU2D_CommandListCpltCallback);
#endif

    error_code = tx_semaphore_create(&nema_irq_sem, (CHAR*)"nema sema irq", 0);
    assert(error_code == TX_SUCCESS);

    error_code = tsi_malloc_init_pool_aligned(0, (void*)nemagfx_pool_mem, (uintptr_t)nemagfx_pool_mem, NEMAGFX_MEM_POOL_SIZE, 1, 8);
    assert(error_code == 0);

    ring_buffer_str.bo = nema_buffer_create(RING_SIZE);
    assert(ring_buffer_str.bo.base_virt);

    error_code = nema_rb_init(&ring_buffer_str, 1);
    if (error_code < 0)
    {
        return error_code;
    }

    last_cl_id = 0;

    return error_code;
}

uint32_t nema_reg_read(uint32_t reg)
{
    return HAL_GPU2D_ReadRegister(&hgpu2d, reg);
}

void nema_reg_write(uint32_t reg, uint32_t value)
{
    HAL_GPU2D_WriteRegister(&hgpu2d, reg, value);
}

int nema_wait_irq(void)
{
    tx_semaphore_get(&nema_irq_sem, TX_WAIT_FOREVER);

    return 0;
}

int nema_wait_irq_cl(int cl_id)
{
    while (last_cl_id < cl_id)
    {
        (void)nema_wait_irq();
    }

    return 0;
}

int nema_wait_irq_brk(int brk_id)
{
    UNUSED(brk_id);

    while (nema_reg_read(GPU2D_BREAKPOINT) == 0U)
    {
        (void)nema_wait_irq();
    }

    return 0;
}

void nema_host_free(void* ptr)
{
    tsi_free(ptr);
}

void* nema_host_malloc(unsigned size)
{
    return tsi_malloc(size);
}

nema_buffer_t nema_buffer_create(int size)
{
    nema_buffer_t bo;

    memset(&bo, 0, sizeof(bo));
    bo.base_virt = tsi_malloc(size);
    bo.base_phys = (uint32_t)bo.base_virt;
    bo.size      = size;
    assert(bo.base_virt != 0 && "Unable to allocate memory in nema_buffer_create");

    return bo;
}

nema_buffer_t nema_buffer_create_pool(int pool, int size)
{
    UNUSED(pool);

    return nema_buffer_create(size);
}

void* nema_buffer_map(nema_buffer_t* bo)
{
    return bo->base_virt;
}

void nema_buffer_unmap(nema_buffer_t* bo)
{
    UNUSED(bo);
}

void nema_buffer_destroy(nema_buffer_t* bo)
{
    if (bo->fd == -1)
    {
        return;
    }

    tsi_free(bo->base_virt);

    bo->base_virt = (void*)0;
    bo->base_phys = 0;
    bo->size      = 0;
    bo->fd        = -1;
}

uintptr_t nema_buffer_phys(nema_buffer_t* bo)
{
    return bo->base_phys;
}

void nema_buffer_flush(nema_buffer_t* bo)
{
    SCB_CleanInvalidateDCache_by_Addr((uint32_t*)bo->base_virt, bo->size);
}

int nema_mutex_lock(int mutex_id)
{
    UNUSED(mutex_id);

    return 0;
}

int nema_mutex_unlock(int mutex_id)
{
    UNUSED(mutex_id);

    return 0;
}
