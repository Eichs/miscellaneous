package main

import (
	"fmt"
	"io/ioutil"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
)

func main() {
	protoPath := "proto"
	outDirs := map[string]string{
		"go":   "out/go",
		"py":   "out/py",
		"java": "out/java",
		"cpp":  "out/cpp",
	}

	// 创建输出目录
	for _, dir := range outDirs {
		if err := os.MkdirAll(dir, 0755); err != nil {
			panic(err)
		}
	}

	// 遍历.proto文件并执行protoc命令
	filepath.Walk(protoPath, func(path string, info os.FileInfo, err error) error {
		if err != nil {
			return err
		}
		if !info.IsDir() && strings.HasSuffix(path, ".proto") {
			for lang, outDir := range outDirs {
				exec.Command("./protoc", fmt.Sprintf("--%s_out=%s", lang, outDir), "--proto_path="+protoPath, path).Run()
			}
		}
		return nil
	})

	goFiles, err := filepath.Glob(outDirs["go"] + "/proto/*.go")
	if err != nil {
		panic(err)
	}
	for _, goFile := range goFiles {
		fmt.Println("Processing file:", goFile) // 打印正在处理的文件名

		content, err := ioutil.ReadFile(goFile)
		if err != nil {
			panic(err)
		}

		// 替换第一个字符串
		newContent := strings.ReplaceAll(string(content), "google.golang.org/protobuf/reflect/protoreflect", "hk4e/framework/common/protobuf/reflect/protoreflect")

		// 替换第二个字符串
		newContent = strings.ReplaceAll(newContent, "google.golang.org/protobuf/runtime/protoimpl", "hk4e/framework/common/protobuf/runtime/protoimpl")
		newContent = strings.ReplaceAll(newContent, "timestamp \"github.com/golang/protobuf/ptypes/timestamp\"", "\"hk4e/framework/common/protobuf/types/timestamp\"")

		if newContent == string(content) {
			fmt.Println("No replacement made in file:", goFile) // 如果没有替换发生
		} else {
			fmt.Println("Replacement made in file:", goFile) // 如果发生了替换
		}

		if err := ioutil.WriteFile(goFile, []byte(newContent), 0644); err != nil {
			panic(err)
		}
	}

	fmt.Println("Done")

}
