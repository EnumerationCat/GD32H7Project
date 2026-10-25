#pragma once

/**
 * @brief 中断向量函数类型定义
 */
typedef void (*VectorType)(void);

/**
 * @brief 外部声明中断向量表
 */
extern const VectorType __Vectors[];