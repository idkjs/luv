/* This file is part of Luv, released under the MIT license. See LICENSE.md for
   details, or visit https://github.com/aantron/luv/blob/master/LICENSE.md. */



open Promise.PipeFirst;



let () = {
  /* Process command-line arguments. */

  let url =
    try (Sys.argv[1]) {
    | Invalid_argument(_) =>
      Printf.eprintf("Usage: %s URL [PATH]\n", Filename.basename(Sys.argv[0]));
      exit(1);
    };
  let path =
    try (Sys.argv[2]) {
    | Invalid_argument(_) => "/"
    };


  /* Do a DNS lookup on the URL we are asked to retrieve. */

  Luv.Promise.DNS.getaddrinfo(~family=`INET, ~node=url, ~service="80", ())
  ->Promise.tapError(error => {
    Printf.eprintf(
      "Could not resolve %s: %s\n", url, Luv.Error.strerror(error));
    exit(1)
  })
  ->Promise.getOk(addr_infos => {
    let addr_info =
      switch (addr_infos) {
      | [] =>
        /* Not sure if Result.Ok is actually possible with the empty list. */
        Printf.eprintf("Could not resolve %s\n", url);
        exit(1);
      | [first, ..._] => first;
      };


    /* Prepare the request text. */

    let request =
      Printf.sprintf("GET %s HTTP/1.1\r\nConnection: close\r\n\r\n", path);


    /* Create the TCP socket and connect it. */

    let socket =
      switch (Luv.TCP.init()) {
      | Result.Ok(socket) => socket
      | Result.Error(error) =>
        Printf.eprintf(
          "Could not create TCP socket: %s\n", Luv.Error.strerror(error));
        exit(1);
      };
    Luv.Promise.TCP.connect(socket, Luv.DNS.Addr_info.(addr_info.addr))
    ->Promise.flatMap(result => {
      switch (result) {
      | Result.Ok() => ()
      | Result.Error(error) =>
        Printf.eprintf(
          "Could not connect to %s: %s\n", url, Luv.Error.strerror(error));
        exit(1);
      };


      /* Write the GET request. */

      Luv.Promise.Stream.write(socket, [Luv.Buffer.from_string(request)])
    })
    ->Promise.get(((result, written)) => {
      switch (result) {
      | Result.Ok() => ()
      | Result.Error(error) =>
        Printf.eprintf(
          "Could not send request: %s", Luv.Error.strerror(error));
        exit(1);
      };
      if (written != String.length(request)) {
        prerr_endline("Could not send request");
        exit(1);
      };


      /* Read the response until the server closes the socket. */

      Luv.Stream.read_start(socket, result =>
        switch (result) {
        | Result.Ok(buffer) =>
          print_string(Luv.Buffer.to_string(buffer))
        | Result.Error(`EOF) =>
          Luv.Handle.close(socket, ignore)
        | Result.Error(error) =>
          Printf.eprintf(
            "Could not read from socket: %s", Luv.Error.strerror(error));
          exit(1);
        });
    });
  });


  /* Wait for all the I/O to finish. The calls to select, epoll, kevent, etc.,
     happen here. */

  Luv.Promise.run();
};
