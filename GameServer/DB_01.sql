-- 현재 DB 버전과 서버 버전이 매칭되는지 확인 후, 맞지 않으면 크래시를 때린다.
-- but, 너무 복잡함.

-- DB 버전 관리를 위한 table
CREATE TABLE [dbo].[Version]
(
    [version] FLOAT NOT NULL
);

CREATE TABLE [dbo].[Gold]
(
    [id] INT NOT NULL PRIMARY KEY IDENTITY,
    [gold] INT NULL,
    [name] NVARCHAR(50) NULL,
    [createDate] DATETIME NULL
);