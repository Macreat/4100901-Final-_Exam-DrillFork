# 4100901-Final_Exam-Drill

Please fork this repo, then clone it into your computer, and place your work for the Final exam here.

## Criterio de Evaluación

#change proof on readme

- Funcionalidad: 80% (valor equivalente para cada requerimiento)
- Repositorio: 10% (commits entendibles que describan los cambios realizados)
- Documentación: 10% (en código y readme del repositorio)

## Instrucciones

Implemente un **sistema de seguridad** según los requerimientos descritos.

### Requerimientos del sistema:

#### No funcionales:

1. Tener un LED para indicar el estatus del sistema de seguridad (preferiblemente con libreria).
2. Tener un teclado hexadecimal para ingresar la clave del sistema en dígitos.
3. Tener un puerto de depuración con el PC (USART2 @256000 baudios).
4. Tener un display OLED para mostrar eventos del sistema.

#### Funcionales:

5. El teclado hexadecimal debe recibir la clave en dígitos, que corresponde al documento del estudiante.
6. El display debe mostrar los dígitos que se han ingresado para que el usuario valide la clave en tiempo real.
7. Se debe presionar la tecla '#' para verificar la clave o la tecla '\*' para reiniciar la secuencia de dígitos.
8. El LED debe encender durante 3 segundos, a 4Hz si la clave es incorrecta y a 0Hz si la clave es correcta.
9. El display OLED debe mostrar "Success" o "Error" según el resultado cuando la clave sea verificada.
10. La consola debe mostrar cuando hayan eventos en el sistema.
