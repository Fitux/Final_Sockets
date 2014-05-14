/**
  @file config.h
  @brief Definiciones Generales

  @author Alvaro Parres
  @date Feb/2013

*/

#ifndef DEFAULTS_H
#define DEFAULTS_H

#define BUFFERSIZE					1048576

/**
  * @defgroup Configs Valores de Configuracion
  * @{
*/

#define	CONFIG_LISENT_IFACE			"0.0.0.0" /*< Interfaz para escucar */
#define CONFIG_MAX_CLIENT			5		/*< Maximo de Conexiones Simultaneas */
#define CONFIG_DEFAULT_PORT			8080	/*< Puerto de trabajo */

#define	CONFIG_DEFAULT_COUNT		5		/*< Numero de Mensajes a enviar */
#define CONFIG_DEFAULT_MSGSIZE		64		/*< Tamaño de Mensaje en KB  */

#define CONFIG_DEFAULT_VERBOSE		1		/*< Nivel de Verbosity  */

/** @}*/


/**
  * @defgroup Constante Constantes
  * @{
*/

#define MINPORT						1025	/*< Menor puerto a Utilizar */
#define MAXPORT						USHRT_MAX /*< Mayor puerto a Utilizar */
/** @}*/

#endif
