(* This file is part of Luv, released under the MIT license. See LICENSE.md for
   details, or visit https://github.com/aantron/luv/blob/master/LICENSE.md. *)



type t = [ `Pipe ] Stream.t

module Mode :
sig
  type t = [
    | `READABLE
    | `WRITABLE
  ]
end

(* DOC Document that the pipe is not yet usable at this point. *)
val init :
  ?loop:Loop.t -> ?for_handle_passing:bool -> unit -> (t, Error.t) result
val open_ : t -> File.t -> (unit, Error.t) result
val bind : t -> string -> (unit, Error.t) result
val connect : t -> string -> ((unit, Error.t) result -> unit) -> unit
val getsockname : t -> (string, Error.t) result
val getpeername : t -> (string, Error.t) result
val pending_instances : t -> int -> unit
val chmod : t -> Mode.t list -> (unit, Error.t) result

(* DOC This absolutely requires documentation to be usable. *)
val receive_handle :
  t -> [
    | `TCP of (TCP.t -> (unit, Error.t) result)
    | `Pipe of (t -> (unit, Error.t) result)
    | `None
  ]

(* DOC Stream.listen is to be used for listening. *)
(* DOC it's not necessary to manually call unlink. close unlinks the file. *)
